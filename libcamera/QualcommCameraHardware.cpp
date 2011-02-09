/*
** Copyright 2008, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

//#define LOG_NDEBUG 0
#define LOG_TAG "QualcommCameraHardware"
#include <utils/Log.h>

#include "QualcommCameraHardware.h"

#include <utils/threads.h>
#include <binder/MemoryHeapPmem.h>
#include <utils/String16.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#if HAVE_ANDROID_OS
#include <linux/android_pmem.h>
#endif
#include <linux/ioctl.h>
#include "raw2jpeg.h"

#define LIKELY(exp)   __builtin_expect(!!(exp), 1)
#define UNLIKELY(exp) __builtin_expect(!!(exp), 0)

extern "C" {
#include "exifwriter.h"

#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdlib.h>
#include <poll.h>

#include "msm_camera.h" // HTC kernel header

#define THUMBNAIL_WIDTH        512 // got from Donut locat
#define THUMBNAIL_HEIGHT       384
#define THUMBNAIL_WIDTH_STR   "512"
#define THUMBNAIL_HEIGHT_STR  "384"
#define DEFAULT_PICTURE_WIDTH  2048
#define DEFAULT_PICTURE_HEIGHT 1536
#define THUMBNAIL_BUFFER_SIZE (THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 3/2)
#define DEFAULT_PREVIEW_SETTING 5 // QVGA
#define DEFAULT_FRAMERATE 15
#define PREVIEW_SIZE_COUNT (sizeof(preview_sizes)/sizeof(preview_size_type))

#define NOT_FOUND -1

#if DLOPEN_LIBMMCAMERA
#include <dlfcn.h>

void* (*LINK_cam_conf)(void *data);
void* (*LINK_cam_frame)(void *data);
bool  (*LINK_jpeg_encoder_init)();
void  (*LINK_jpeg_encoder_join)();
unsigned char (*LINK_jpeg_encoder_encode)(const char* file_name, const cam_ctrl_dimension_t *dimen,
                                  const unsigned char* thumbnailbuf, int thumbnailfd,
                                  const unsigned char* snapshotbuf, int snapshotfd, common_crop_t *cropInfo);
int  (*LINK_camframe_terminate)(void);
int8_t (*LINK_jpeg_encoder_setMainImageQuality)(uint32_t quality);
// Tattoo
int8_t (*LINK_jpeg_encoder_setThumbnailQuality)(uint32_t quality);
int8_t (*LINK_jpeg_encoder_setRotation)(uint32_t rotation);
int8_t (*LINK_jpeg_encoder_setLocation)(const camera_position_type *location);
//
// callbacks
void  (**LINK_mmcamera_camframe_callback)(struct msm_frame_t *frame);
void  (**LINK_mmcamera_jpegfragment_callback)(uint8_t *buff_ptr,
                                              uint32_t buff_size);
void  (**LINK_mmcamera_jpeg_callback)(jpeg_event_t status);
#else
#define LINK_cam_conf cam_conf
#define LINK_cam_frame cam_frame
#define LINK_jpeg_encoder_init jpeg_encoder_init
#define LINK_jpeg_encoder_join jpeg_encoder_join
#define LINK_jpeg_encoder_encode jpeg_encoder_encode
#define LINK_camframe_terminate camframe_terminate
#define LINK_jpeg_encoder_setMainImageQuality jpeg_encoder_setMainImageQuality
#define LINK_jpeg_encoder_setThumbnailQuality jpeg_encoder_setThumbnailQuality
#define LINK_jpeg_encoder_setRotation jpeg_encoder_setRotation
#define LINK_jpeg_encoder_setLocation jpeg_encoder_setLocation
extern void (*mmcamera_camframe_callback)(struct msm_frame_t *frame);
extern void (*mmcamera_jpegfragment_callback)(uint8_t *buff_ptr,
                                      uint32_t buff_size);
extern void (*mmcamera_jpeg_callback)(jpeg_event_t status);
#endif

} // extern "C"

struct preview_size_type {
    int width;
    int height;
};

static preview_size_type preview_sizes[] = {
    { 800, 480 }, // WVGA
    { 640, 480 }, // VGA
    { 480, 320 }, // HVGA
    { 384, 288 },
    { 352, 288 }, // CIF
    { 320, 240 }, // QVGA
    { 240, 160 }, // SQVGA
    { 176, 144 }, // QCIF
    { 192, 144 }, // Donut Tattoo
    { 252, 189 },  // Donut Tattoo
};

static int attr_lookup(const struct str_map *const arr, const char *name)
{
    if (name) {
        const struct str_map *trav = arr;
        while (trav->desc) {
            if (!strcmp(trav->desc, name))
                return trav->val;
            trav++;
        }
    }
    return NOT_FOUND;
}

#define INIT_VALUES_FOR(parm) do {                               \
    if (!parm##_values) {                                        \
        parm##_values = (char *)malloc(sizeof(parm)/             \
                                       sizeof(parm[0])*30);      \
        char *ptr = parm##_values;                               \
        const str_map *trav;                                     \
        for (trav = parm; trav->desc; trav++) {                  \
            int len = strlen(trav->desc);                        \
            strcpy(ptr, trav->desc);                             \
            ptr += len;                                          \
            *ptr++ = ',';                                        \
        }                                                        \
        *--ptr = 0;                                              \
    }                                                            \
} while(0)

// from aeecamera.h
static const str_map whitebalance[] = {
    { "auto",         CAMERA_WB_AUTO },
    { "incandescent", CAMERA_WB_INCANDESCENT },
    { "fluorescent",  CAMERA_WB_FLUORESCENT },
    { "daylight",     CAMERA_WB_DAYLIGHT },
    { "cloudy",       CAMERA_WB_CLOUDY_DAYLIGHT },
    { "twilight",     CAMERA_WB_TWILIGHT },
    { "shade",        CAMERA_WB_SHADE },
    { NULL, 0 }
};
static char *whitebalance_values;

// from camera_effect_t
static const str_map effect[] = {
    { "none",       CAMERA_EFFECT_OFF },  /* This list must match aeecamera.h */
    { "mono",       CAMERA_EFFECT_MONO },
    { "negative",   CAMERA_EFFECT_NEGATIVE },
    { "solarize",   CAMERA_EFFECT_SOLARIZE },
    { "sepia",      CAMERA_EFFECT_SEPIA },
    { "posterize",  CAMERA_EFFECT_POSTERIZE },
    { "whiteboard", CAMERA_EFFECT_WHITEBOARD },
    { "blackboard", CAMERA_EFFECT_BLACKBOARD },
    { "aqua",       CAMERA_EFFECT_AQUA },
    { NULL, 0 }
};
static char *effect_values;

// from qcamera/common/camera.h
static const str_map antibanding[] = {
    { "off",  CAMERA_ANTIBANDING_OFF },
    { "50hz",  CAMERA_ANTIBANDING_50HZ },
    { "60hz",  CAMERA_ANTIBANDING_60HZ },
    { "auto",  CAMERA_ANTIBANDING_AUTO },
    { NULL, 0 }
};
static char *antibanding_values;

// round to the next power of two
static inline unsigned clp2(unsigned x)
{
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return x + 1;
}

namespace android {

static Mutex singleton_lock;
static bool singleton_releasing;
static Condition singleton_wait;

static void receive_camframe_callback(struct msm_frame_t *frame);
static void receive_jpeg_fragment_callback(uint8_t *buff_ptr, uint32_t buff_size);
static void receive_jpeg_callback(jpeg_event_t status);

static int camerafd;
pthread_t w_thread;
pthread_t jpegThread ;


void *opencamerafd(void *arg) {
    camerafd = open(MSM_CAMERA_CONTROL, O_RDWR);
    if (camerafd < 0)
        LOGE("Camera control %s open failed: %s!", MSM_CAMERA_CONTROL, strerror(errno));
    else
        LOGV("opening %s fd: %d", MSM_CAMERA_CONTROL, camerafd);

    return NULL;
}

QualcommCameraHardware::QualcommCameraHardware()
    : mParameters(),
      mPreviewHeight(-1),
      mPreviewWidth(-1),
      mRawHeight(-1),
      mRawWidth(-1),
      mCameraRunning(false),
      mPreviewInitialized(false),
      // mRawInitialized(false),
      mFrameThreadRunning(false),
      mSnapshotThreadRunning(false),
      mReleasedRecordingFrame(false),
      mNotifyCb(0),
      mDataCb(0),
      mDataCbTimestamp(0),
      mCallbackCookie(0),
      mMsgEnabled(0),
      mPreviewFrameSize(0),
      mRawSize(0),
      mCameraControlFd(-1),
      mAutoFocusThreadRunning(false),
      mAutoFocusFd(-1),
      mInPreviewCallback(false),
      mCameraRecording(false)
{
    if((pthread_create(&w_thread, NULL, opencamerafd, NULL)) != 0) {
        LOGE("Camera open thread creation failed") ;
    }
    memset(&mDimension, 0, sizeof(mDimension));
    memset(&mCrop, 0, sizeof(mCrop));
    LOGV("constructor EX");
}

void QualcommCameraHardware::initDefaultParameters()
{
    CameraParameters p;

    LOGV("initDefaultParameters E");

    preview_size_type *ps = &preview_sizes[DEFAULT_PREVIEW_SETTING];
    p.setPreviewSize(ps->width, ps->height);

    p.setPreviewFrameRate(DEFAULT_FRAMERATE);
    p.setPreviewFormat(CameraParameters::PIXEL_FORMAT_YUV420SP); // informative
    p.setPictureFormat(CameraParameters::PIXEL_FORMAT_JPEG); // informative

    p.set("jpeg-quality", "100"); // maximum quality
    p.set("jpeg-thumbnail-width", THUMBNAIL_WIDTH_STR); // informative
    p.set("jpeg-thumbnail-height", THUMBNAIL_HEIGHT_STR); // informative
    p.set("jpeg-thumbnail-quality", "85");


    p.setPictureSize(DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
    p.set(CameraParameters::KEY_ANTIBANDING, CameraParameters::ANTIBANDING_OFF);
    p.set(CameraParameters::KEY_EFFECT, CameraParameters::EFFECT_NONE);
    p.set(CameraParameters::KEY_WHITE_BALANCE, CameraParameters::WHITE_BALANCE_AUTO);

#if 0
    p.set("gps-timestamp", "1199145600"); // Jan 1, 2008, 00:00:00
    p.set("gps-latitude", "37.736071"); // A little house in San Francisco
    p.set("gps-longitude", "-122.441983");
    p.set("gps-altitude", "21"); // meters
#endif

    // This will happen only one in the lifetime of the mediaserver process.
    // We do not free the _values arrays when we destroy the camera object.
    INIT_VALUES_FOR(antibanding);
    INIT_VALUES_FOR(effect);
    INIT_VALUES_FOR(whitebalance);

    // Create de supported camera values
    p.set(CameraParameters::KEY_SUPPORTED_ANTIBANDING, antibanding_values);
    p.set(CameraParameters::KEY_SUPPORTED_EFFECTS, effect_values);
    p.set(CameraParameters::KEY_SUPPORTED_WHITE_BALANCE, whitebalance_values);

    // Set display camera supported sizes
    // -----------------------------------------------------
    p.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, "2048x1536,1600x1200,1024x768");
    p.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, "512x348,320x240,192x144,252x189");

    if (setParameters(p) != NO_ERROR) {
        LOGE("Failed to set default parameters?!");
    }

    LOGV("initDefaultParameters X");

}

void QualcommCameraHardware::setCallbacks(notify_callback notify_cb,
                                      data_callback data_cb,
                                      data_callback_timestamp data_cb_timestamp,
                                      void* user)
{
    Mutex::Autolock lock(mLock);
    mNotifyCb = notify_cb;
    mDataCb = data_cb;
    mDataCbTimestamp = data_cb_timestamp;
    mCallbackCookie = user;
}

void QualcommCameraHardware::enableMsgType(int32_t msgType)
{
    Mutex::Autolock lock(mLock);
    LOGD("enableMsgType( %d )", msgType);
    mMsgEnabled |= msgType;
}

void QualcommCameraHardware::disableMsgType(int32_t msgType)
{
    Mutex::Autolock lock(mLock);
    LOGD("DisableMsgType( %d )", msgType);
    mMsgEnabled &= ~msgType;
}

bool QualcommCameraHardware::msgTypeEnabled(int32_t msgType)
{
    Mutex::Autolock lock(mLock);
    LOGD("msgTypeEnabled( %d )", msgType);
    return (mMsgEnabled & msgType);
}


#define ROUND_TO_PAGE(x)  (((x)+0xfff)&~0xfff)

void QualcommCameraHardware::startCamera()
{
    unsigned char sync_value;

    LOGV("startCamera E");
#if DLOPEN_LIBMMCAMERA
    libmmcamera = ::dlopen("libmmcamera.so", RTLD_NOW);
    LOGV("loading libmmcamera at %p", libmmcamera);
    if (!libmmcamera) {
        LOGE("FATAL ERROR: could not dlopen liboemcamera.so: %s", dlerror());
        return;
    }

    libmmcamera_target = ::dlopen("libmm-qcamera-tgt.so", RTLD_NOW);
    LOGV("loading libmm-qcamera-tgt at %p", libmmcamera_target);
    if (!libmmcamera_target) {
        LOGE("FATAL ERROR: could not dlopen libmm-qcamera_target.so: %s", dlerror());
        return;
    }

    *(void **)&LINK_cam_frame =
        ::dlsym(libmmcamera, "cam_frame");

    *(void **)&LINK_camframe_terminate =
        ::dlsym(libmmcamera, "camframe_terminate");

    *(void **)&LINK_jpeg_encoder_init =
        ::dlsym(libmmcamera, "jpeg_encoder_init");

    *(void **)&LINK_jpeg_encoder_encode =
        ::dlsym(libmmcamera, "jpeg_encoder_encode");

    *(void **)&LINK_jpeg_encoder_join =
        ::dlsym(libmmcamera, "jpeg_encoder_join");

    *(void **)&LINK_mmcamera_camframe_callback =
        ::dlsym(libmmcamera, "mmframe_cb");

    *LINK_mmcamera_camframe_callback = receive_camframe_callback;

    *(void **)&LINK_mmcamera_jpegfragment_callback =
        ::dlsym(libmmcamera, "mm_jpegfragment_callback");

    *LINK_mmcamera_jpegfragment_callback = receive_jpeg_fragment_callback;

    *(void **)&LINK_mmcamera_jpeg_callback =
        ::dlsym(libmmcamera, "mm_jpeg_callback");

    *LINK_mmcamera_jpeg_callback = receive_jpeg_callback;

    *(void**)&LINK_jpeg_encoder_setMainImageQuality =
        ::dlsym(libmmcamera, "jpeg_encoder_setMainImageQuality");

    *(void **)&LINK_cam_conf =
        ::dlsym(libmmcamera_target, "cam_conf");
#else
    mmcamera_camframe_callback = receive_camframe_callback;
    mmcamera_jpegfragment_callback = receive_jpeg_fragment_callback;
    mmcamera_jpeg_callback = receive_jpeg_callback;
#endif // DLOPEN_LIBMMCAMERA

    /* The control thread is in libcamera itself. */
    LOGV("pthread_join on control thread");
    if (pthread_join(w_thread, NULL) != 0) {
        LOGE("Camera open thread exit failed");
        return;
    }

    mCameraControlFd = camerafd;

    if (!LINK_jpeg_encoder_init()) {
        LOGE("jpeg_encoding_init failed.\n");
    }

    if ((pthread_create(&mCamConfigThread, NULL, LINK_cam_conf, NULL)) != 0)
        LOGE("Config thread creation failed!");
    else
        LOGV("cam_conf thread created");

    LOGV("startCamera X");
}

status_t QualcommCameraHardware::dump(int fd,
                                      const Vector<String16>& args) const
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    // Dump internal primitives.
    result.append("QualcommCameraHardware::dump");
    snprintf(buffer, 255, "preview width(%d) x height (%d)\n",
             mPreviewWidth, mPreviewHeight);
    result.append(buffer);
    snprintf(buffer, 255, "raw width(%d) x height (%d)\n",
             mRawWidth, mRawHeight);
    result.append(buffer);
    snprintf(buffer, 255,
             "preview frame size(%d), raw size (%d), jpeg size (%d) "
             "and jpeg max size (%d)\n", mPreviewFrameSize, mRawSize,
             mJpegSize, mJpegMaxSize);
    result.append(buffer);
    write(fd, result.string(), result.size());

    // Dump internal objects.
    if (mPreviewHeap != 0) {
        mPreviewHeap->dump(fd, args);
    }
    if (mRawHeap != 0) {
        mRawHeap->dump(fd, args);
    }
    if (mJpegHeap != 0) {
        mJpegHeap->dump(fd, args);
    }
    mParameters.dump(fd, args);
    return NO_ERROR;
}

static bool native_set_afmode(int camfd, isp3a_af_mode_t af_type)
{
    int rc;
    struct msm_ctrl_cmd_t ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type = CAMERA_SET_PARM_AUTO_FOCUS;
    ctrlCmd.length = sizeof(af_type);
    ctrlCmd.value = &af_type;

    if ((rc = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0)
        LOGE("native_set_afmode: ioctl fd %d error %s\n",
             camfd,
             strerror(errno));

    LOGV("native_set_afmode: ctrlCmd.status == %d\n", ctrlCmd.status);
    return rc >= 0 && ctrlCmd.status == CAMERA_EXIT_CB_DONE;
}

static bool native_cancel_afmode(int camfd, int af_fd)
{
    int rc;
    struct msm_ctrl_cmd_t ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type = CAMERA_AUTO_FOCUS_CANCEL;
    ctrlCmd.length = 0;

    if ((rc = ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd)) < 0)
        LOGE("native_cancel_afmode: ioctl fd %d error %s\n",
             camfd,
             strerror(errno));
    return rc >= 0;
}

static bool native_start_preview(int camfd)
{
    struct msm_ctrl_cmd_t ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_START_PREVIEW;
    ctrlCmd.length     = 0;

    LOGV("Kalim: Iniciado Start_preview EEEEEEEEEEEEEEEEEEEEEE");

    if (ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        LOGE("native_start_preview: MSM_CAM_IOCTL_CTRL_COMMAND fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }
    LOGV("Kalim: Finalizado Start_preview XXXXXXXXXXXXXXXXXXXX");

    return true;
}

static bool native_get_picture(int camfd, common_crop_t *crop)
{
    struct msm_ctrl_cmd_t ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.length     = sizeof(common_crop_t);
    ctrlCmd.value      = crop;

    if(ioctl(camfd, MSM_CAM_IOCTL_GET_PICTURE, &ctrlCmd) < 0) {
        LOGE("native_get_picture: MSM_CAM_IOCTL_GET_PICTURE fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    LOGV("crop: in1_w %d", crop->in1_w);
    LOGV("crop: in1_h %d", crop->in1_h);
    LOGV("crop: out1_w %d", crop->out1_w);
    LOGV("crop: out1_h %d", crop->out1_h);

    LOGV("crop: in2_w %d", crop->in2_w);
    LOGV("crop: in2_h %d", crop->in2_h);
    LOGV("crop: out2_w %d", crop->out2_w);
    LOGV("crop: out2_h %d", crop->out2_h);

    LOGV("crop: update %d", crop->update_flag);


    return true;
}

static bool native_stop_preview(int camfd)
{
    struct msm_ctrl_cmd_t ctrlCmd;
    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_STOP_PREVIEW;
    ctrlCmd.length     = 0;

    LOGV("Kalim: Iniciado Stop_preview EEEEEEEEEEEEEEEEEEEEEE");

    if(ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        LOGE("native_stop_preview: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }
    LOGV("Kalim: Finalizado Start_preview XXXXXXXXXXXXXXXXXXXXXX");

    return true;
}

static bool native_start_snapshot(int camfd)
{
    struct msm_ctrl_cmd_t ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_START_SNAPSHOT;
    ctrlCmd.length     = 0;

    if(ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        LOGE("native_start_snapshot: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    return true;
}

static bool native_stop_snapshot (int camfd)
{
    struct msm_ctrl_cmd_t ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = CAMERA_STOP_SNAPSHOT;
    ctrlCmd.length     = 0;

    if (ioctl(camfd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0) {
        LOGE("native_stop_snapshot: ioctl fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }

    return true;
}

void *jpeg_encoder_thread( void *user )
{
  LOGD("jpeg_encoder_thread E");
    sp<QualcommCameraHardware> obj = QualcommCameraHardware::getInstance();
    if (obj != 0) {
        obj->runJpegEncodeThread(user);
    }
    else LOGW("not starting frame thread: the object went away!");
    LOGD("jpeg_encoder_thread X");
    return NULL;
}


bool QualcommCameraHardware::native_jpeg_encode(void)
{
    int jpeg_quality = mParameters.getInt("jpeg-quality");
    if (jpeg_quality >= 0) {
        LOGV("native_jpeg_encode, current jpeg main img quality =%d",
             jpeg_quality);
        if(!LINK_jpeg_encoder_setMainImageQuality(jpeg_quality)) {
            LOGE("native_jpeg_encode set jpeg-quality failed");
            return false;
        }
    }

    int thumbnail_quality = mParameters.getInt("jpeg-thumbnail-quality");
    if (thumbnail_quality >= 0) {
        LOGV("native_jpeg_encode, current jpeg thumbnail quality =%d",
             thumbnail_quality);
    }

    int rotation = mParameters.getInt("rotation");
    if (rotation >= 0) {
        LOGV("native_jpeg_encode, rotation = %d", rotation);
    }
    char jpegFileName[256] = {0};
    static int snapshotCntr = 0;

    mDimension.filler7 = 2560 ;
    mDimension.filler8 = 1920 ;


    // ******************************** SACADO DE NOPY *********************************************
    LOGV("picture_width %d, picture_height = %d, display_width = %d, display_height = %d, filler = %d, filler2 = %d, ui_thumbnail_height = %d , ui_thumbnail_width = %d, filler3 = %d, filler4 = %d, filler5 = %d, filler6 = %d, filler7 = %d, filler8 = %d\n" ,
                 mDimension.picture_width,mDimension.picture_height,
                 mDimension.display_width,mDimension.display_height,
                 mDimension.filler, mDimension.filler2,
                 mDimension.ui_thumbnail_height, mDimension.ui_thumbnail_width,
                 mDimension.filler3, mDimension.filler4, mDimension.filler5, mDimension.filler6,
                 mDimension.filler7, mDimension.filler8 );

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


    LOGI("KALIM: Lanzamos inicio de conversion");
    int ret = !pthread_create(&jpegThread,
                              &attr, //NULL,
                              jpeg_encoder_thread,
                              NULL);
    // *********************************************************************************************

    return true;
}

bool QualcommCameraHardware::native_set_dimension(cam_ctrl_dimension_t *value)
{
    LOGV("native_set_dimension: E");
    return native_set_parm(CAMERA_SET_PARM_DIMENSION,
                           sizeof(cam_ctrl_dimension_t), value);
    LOGV("native_set_dimension: X");
}

bool QualcommCameraHardware::native_set_parm(
    cam_ctrl_type type, uint16_t length, void *value)
{
    int rc = true;
    struct msm_ctrl_cmd_t ctrlCmd;

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.type       = (uint16_t)type;
    ctrlCmd.length     = length;
    ctrlCmd.value = value;

    LOGV("native_set_parm: type: %d, length=%d", type, length);

    rc = ioctl(mCameraControlFd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd);
    if(rc < 0 || ctrlCmd.status != CAM_CTRL_SUCCESS) {
        LOGE("ioctl error. camfd=%d, type=%d, length=%d, rc=%d, ctrlCmd.status=%d, %s",
             mCameraControlFd, type, length, rc, ctrlCmd.status, strerror(errno));
        return false;
    }
    return true;
}

void QualcommCameraHardware::jpeg_set_location()
{
    bool encode_location = true;
    camera_position_type pt;

#define PARSE_LOCATION(what,type,fmt,desc) do {                                \
        pt.what = 0;                                                           \
        const char *what##_str = mParameters.get("gps-"#what);                 \
        LOGV("GPS PARM %s --> [%s]", "gps-"#what, what##_str);                 \
        if (what##_str) {                                                      \
            type what = 0;                                                     \
            if (sscanf(what##_str, fmt, &what) == 1)                           \
                pt.what = what;                                                \
            else {                                                             \
                LOGE("GPS " #what " %s could not"                              \
                     " be parsed as a " #desc, what##_str);                    \
                encode_location = false;                                       \
            }                                                                  \
        }                                                                      \
        else {                                                                 \
            LOGV("GPS " #what " not specified: "                               \
                 "defaulting to zero in EXIF header.");                        \
            encode_location = false;                                           \
       }                                                                       \
    } while(0)

    PARSE_LOCATION(timestamp, long, "%ld", "long");
    if (!pt.timestamp) pt.timestamp = time(NULL);
    PARSE_LOCATION(altitude, short, "%hd", "short");
    PARSE_LOCATION(latitude, double, "%lf", "double float");
    PARSE_LOCATION(longitude, double, "%lf", "double float");

#undef PARSE_LOCATION

    if (encode_location) {
        LOGD("setting image location ALT %d LAT %lf LON %lf",
             pt.altitude, pt.latitude, pt.longitude);
    }
    else LOGV("not setting image location");
}

void QualcommCameraHardware::runFrameThread(void *data)
{
    LOGV("runFrameThread E");

#if DLOPEN_LIBMMCAMERA
    // We need to maintain a reference to libmmcamera.so for the duration of the
    // frame thread, because we do not know when it will exit relative to the
    // lifetime of this object.  We do not want to dlclose() libmmcamera while
    // LINK_cam_frame is still running.
    void *libhandle = ::dlopen("libmmcamera.so", RTLD_NOW);
    LOGV("FRAME: loading libmmcamera at %p", libhandle);
    if (!libhandle) {
        LOGE("FATAL ERROR: could not dlopen libmmcamera.so: %s", dlerror());
    }
    if (libhandle)
#endif
    {
        LOGV("Before LINK_cam_frame");
        LINK_cam_frame(data);
        LOGV("After LINK_cam_frame");
    }

#if DLOPEN_LIBMMCAMERA
    if (libhandle) {
        ::dlclose(libhandle);
        LOGV("FRAME: dlclose(libmmcamera)");
    }
#endif

    mFrameThreadWaitLock.lock();
    mFrameThreadRunning = false;
    mFrameThreadWait.signal();
    mFrameThreadWaitLock.unlock();

    LOGV("runFrameThread X");
}

void QualcommCameraHardware::runJpegEncodeThread(void *data)
{
    unsigned char *buffer ;

// readFromMemory( (unsigned char *)mRawHeap->mHeap->base(), 2097152, buffer ) ;
// writeToMemory( buffer, 2560, 1920, (unsigned char *)mJpegHeap->mHeap->base(), (int *)&mJpegSize ) ;

    int rotation = mParameters.getInt("rotation");
    LOGD("native_jpeg_encode, rotation = %d", rotation);

    bool encode_location = true;
        camera_position_type pt;

        #define PARSE_LOCATION(what,type,fmt,desc) do { \
                pt.what = 0; \
                const char *what##_str = mParameters.get("gps-"#what); \
                LOGD("GPS PARM %s --> [%s]", "gps-"#what, what##_str); \
                if (what##_str) { \
                        type what = 0; \
                        if (sscanf(what##_str, fmt, &what) == 1) \
                                pt.what = what; \
                        else { \
                                LOGE("GPS " #what " %s could not" \
                                " be parsed as a " #desc, what##_str); \
                                encode_location = false; \
                        } \
                } \
                else { \
                        LOGD("GPS " #what " not specified: " \
                        "defaulting to zero in EXIF header."); \
                        encode_location = false; \
                } \
        } while(0)

    PARSE_LOCATION(timestamp, long, "%ld", "long");
    if (!pt.timestamp) pt.timestamp = time(NULL);
    PARSE_LOCATION(altitude, short, "%hd", "short");
    PARSE_LOCATION(latitude, double, "%lf", "double float");
    PARSE_LOCATION(longitude, double, "%lf", "double float");

        #undef PARSE_LOCATION

    if (encode_location) {
        LOGD("setting image location ALT %d LAT %lf LON %lf",
             pt.altitude, pt.latitude, pt.longitude);
    }
    else {
        LOGV("not setting image location");

    }

    LOGD("mJpegSize %d" , mJpegSize ) ;

    camera_position_type *npt = &pt ;
    if( ! encode_location ) {
        npt = NULL ;
    }

    writeExif( mRawHeap->mHeap->base(), mJpegHeap->mHeap->base(), mJpegSize, &mJpegSize, rotation , npt );

    int jpeg_quality = mParameters.getInt("jpeg-quality");

    LOGV("Kalim: Inicio conversion a JPEG ----------------------------------------------------------------------------------------------");
    LOGV("KalimochoAz jpeg convert, current jpeg main img quality =%d", jpeg_quality);
    LOGV("KalimochoAz jpeg convert, current jpeg main img Height =%d", mRawHeight);
    LOGV("KalimochoAz jpeg convert, current jpeg main img Width =%d", mRawWidth);
    if( yuv420_save2jpeg((unsigned char*) mJpegHeap->mHeap->base(), mRawHeap->mHeap->base(), mRawWidth, mRawHeight, jpeg_quality) )
        LOGV("Kalim: JpegConvetida Correctamente ***********************************************************************************************************");
    else
        LOGV("Kalim: Fallo de conversion a JPEG xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");


    receiveJpegPicture();
}

void *frame_thread(void *user)
{
    LOGV("frame_thread E");
    sp<QualcommCameraHardware> obj = QualcommCameraHardware::getInstance();
//    if (obj != 0) {
//        obj->runFrameThread(user);
//    }
//    else LOGW("not starting frame thread: the object went away!");
    LOGV("frame_thread X");
    return NULL;
}

bool QualcommCameraHardware::initPreview()
{
    // See comments in deinitPreview() for why we have to wait for the frame
    // thread here, and why we can't use pthread_join().
    LOGV("initPreview E: preview size=%dx%d", mPreviewWidth, mPreviewHeight);

    mFrameThreadWaitLock.lock();
    // Kalim FAKE ------------------------ ( si lo elimino no grabamos la foto )
    mFrameThreadRunning = false;
    // -----------------------------------
    while (mFrameThreadRunning) {
        LOGV("initPreview: waiting for old frame thread to complete.");
        mFrameThreadWait.wait(mFrameThreadWaitLock);
        LOGV("initPreview: old frame thread completed.");
    }

    LOGI("KALIM: Wait for unlock frame");
    mFrameThreadWaitLock.unlock();

    LOGI("KALIM: Wait for unlock thread");
    mSnapshotThreadWaitLock.lock();

    while (mSnapshotThreadRunning) {
        LOGI("KALIM: Wait INSIDE for unlock tread to be finished");
        LOGV("initPreview: waiting for old snapshot thread to complete.");
        mSnapshotThreadWait.wait(mSnapshotThreadWaitLock);
        LOGV("initPreview: old snapshot thread completed.");
    }
    LOGI("KALIM: Wait for unlock thread AGAIN");
    mSnapshotThreadWaitLock.unlock();

    mPreviewFrameSize = mPreviewWidth * mPreviewHeight * 3/2; // actual
/*    mPreviewHeap = new PreviewPmemPool(mCameraControlFd,
                                       mPreviewWidth * mPreviewHeight * 2, // worst
                                       kPreviewBufferCount,
                                       mPreviewFrameSize,
                                       0,
                                       "preview");
*/
    mPreviewHeap = new PmemPool(       "/dev/pmem_adsp",
                                       mCameraControlFd,
                                       MSM_PMEM_OUTPUT2,
                                       mPreviewFrameSize,
                                       kPreviewBufferCount,
                                       mPreviewFrameSize,
                                       0,
                                       "preview");

    LOGI("KALIM: try to initialize preview");
    if (!mPreviewHeap->initialized()) {
        LOGE("KALIM: Error at init preview");
        mPreviewHeap.clear();
        LOGE("initPreview X: could not initialize preview heap.");
        return false;
    }

    mDimension.picture_width  = DEFAULT_PICTURE_WIDTH;
    mDimension.picture_height = DEFAULT_PICTURE_HEIGHT;

    bool ret = native_set_dimension(&mDimension);

    if (ret) {
        LOGI("KALIM: dimension SET");
        for (int cnt = 0; cnt < kPreviewBufferCount; cnt++) {
            frames[cnt].fd = mPreviewHeap->mHeap->getHeapID();
            frames[cnt].buffer =
                (uint32_t)mPreviewHeap->mHeap->base() + mPreviewFrameSize * cnt;
            frames[cnt].y_off = 0;
            frames[cnt].cbcr_off = mPreviewWidth * mPreviewHeight;
            frames[cnt].path = MSM_FRAME_ENC;
        }

        mFrameThreadWaitLock.lock();
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        mFrameThreadRunning = !pthread_create(&mFrameThread,
                                              &attr,
                                              frame_thread,
                                              &frames[kPreviewBufferCount-1]);
        ret = mFrameThreadRunning;
        if (ret)
            LOGV("Preview thread created");
        mFrameThreadWaitLock.unlock();
    }

    LOGV("initPreview X");
    return ret;
}

void QualcommCameraHardware::deinitPreview(void)
{
    LOGV("deinitPreview E");

    // When we call deinitPreview(), we signal to the frame thread that it
    // needs to exit, but we DO NOT WAIT for it to complete here.  The problem
    // is that deinitPreview is sometimes called from the frame-thread's
    // callback, when the refcount on the Camera client reaches zero.  If we
    // called pthread_join(), we would deadlock.  So, we just call
    // LINK_camframe_terminate() in deinitPreview(), which makes sure that
    // after the preview callback returns, the camframe thread will exit.  We
    // could call pthread_join() in initPreview() to join the last frame
    // thread.  However, we would also have to call pthread_join() in release
    // as well, shortly before we destoy the object; this would cause the same
    // deadlock, since release(), like deinitPreview(), may also be called from
    // the frame-thread's callback.  This we have to make the frame thread
    // detached, and use a separate mechanism to wait for it to complete.

    /* if (LINK_camframe_terminate() < 0)
        LOGE("failed to stop the camframe thread: %s",
             strerror(errno));
    else
        LOGV("terminate frame_thread successfully");
    */

    LOGV("deinitPreview X");
}

bool QualcommCameraHardware::initRaw(bool initJpegHeap)
{
    LOGV("initRaw E: picture size=%dx%d",
         mRawWidth, mRawHeight); // 2048x1536

    mDimension.picture_width   = mRawWidth;
    mDimension.picture_height  = mRawHeight;
    mRawSize = mRawWidth * mRawHeight * 3 / 2;
    mJpegMaxSize = mRawWidth * mRawHeight * 3 / 2;

    if(!native_set_dimension(&mDimension)) {
        LOGE("initRaw X: failed to set dimension");
        return false;
    }

    if (mJpegHeap != NULL) {
        LOGV("initRaw: clearing old mJpegHeap.");
        mJpegHeap.clear();
    }

/*
 * Order sequence found in Donut logcat:
 * 1: Thumbnail camera backed by pmem pool /dev/msm_adsp
 * 2: Snapshot camera backed by pmem pool /dev/msm_camera
 * 3: Jpeg backed by ashmem
 */

    // Thumbnails

    LOGV("initRaw: initializing mThumbHeap. with size %d", THUMBNAIL_BUFFER_SIZE);
    mThumbnailHeap =
        new PmemPool("/dev/pmem_adsp",
                     mCameraControlFd,
                     MSM_PMEM_THUMBNAIL,
                     THUMBNAIL_BUFFER_SIZE,
                     1,
                     THUMBNAIL_BUFFER_SIZE,
                     0,
                     "thumbnail camera");

    if (!mThumbnailHeap->initialized()) {
        mThumbnailHeap.clear();
        mJpegHeap.clear();
        mRawHeap.clear();
        LOGE("initRaw X failed: error initializing mThumbnailHeap.");
        return false;
    }

    // Snapshot

    LOGV("initRaw: initializing mRawHeap. with size %d", mRawSize); // 4718592
    mRawHeap =
        new PmemPool("/dev/pmem_camera",
                     mCameraControlFd,
                     MSM_PMEM_MAINIMG,
                     mJpegMaxSize,
                     kRawBufferCount,
                     mRawSize,
                     0,
                     "snapshot camera");

    if (!mRawHeap->initialized()) {
        LOGE("initRaw X failed with pmem_camera, trying with pmem_adsp");
        mRawHeap =
            new PmemPool("/dev/pmem_adsp",
                         mCameraControlFd,
                         MSM_PMEM_MAINIMG,
                         mJpegMaxSize,
                         kRawBufferCount,
                         mRawSize,
                         0,
                         "snapshot camera");
        if (!mRawHeap->initialized()) {
            mRawHeap.clear();
            LOGE("initRaw X: error initializing mRawHeap");
            return false;
        }
    }

    LOGV("do_mmap snapshot pbuf = %p, pmem_fd = %d",
         (uint8_t *)mRawHeap->mHeap->base(), mRawHeap->mHeap->getHeapID());

    // Jpeg

    if (initJpegHeap) {
        LOGV("initRaw: initializing mJpegHeap.");
        mJpegHeap =
            new AshmemPool(mJpegMaxSize,
                           kJpegBufferCount,
                           0, // we do not know how big the picture wil be
                           0,
                           "jpeg");

        if (!mJpegHeap->initialized()) {
            mJpegHeap.clear();
            mRawHeap.clear();
            LOGE("initRaw X failed: error initializing mJpegHeap.");
            return false;
        }
    }

    mRawInitialized = true;

    LOGV("initRaw X success");
    return true;
}

void QualcommCameraHardware::deinitRaw()
{
    LOGV("deinitRaw E");
    mThumbnailHeap.clear();
    mJpegHeap.clear();
    mRawHeap.clear();

    mRawInitialized = false;
    LOGV("deinitRaw X");
}

void QualcommCameraHardware::release()
{
    LOGD("release E");
    Mutex::Autolock l(&mLock);

#if DLOPEN_LIBMMCAMERA
    if (libmmcamera == NULL) {
        LOGE("ERROR: multiple release!");
        return;
    }
#else
#warning "Cannot detect multiple release when not dlopen()ing liboemcamera!"
#endif

    int rc;
    struct msm_ctrl_cmd_t ctrlCmd;

    if (mCameraRunning) {
        LOGV("Kalim ----------- mCameraRunning ------------");
        //if (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
            LOGV("Kalim ----------- VIDEO ------------");
            mRecordFrameLock.lock();
            mReleasedRecordingFrame = true;
            mRecordWait.signal();
            mRecordFrameLock.unlock();
        //}
        stopPreviewInternal();
    }

    //FIXME: crash when released
    //LOGV("KALIM Join IMAGE");
    //LINK_jpeg_encoder_join();

    if (mRawInitialized) deinitRaw();

    LOGV("CAMERA_EXIT");

    ctrlCmd.timeout_ms = 5000;
    ctrlCmd.length = 0;
    ctrlCmd.type = (uint16_t)CAMERA_EXIT;

    if (ioctl(mCameraControlFd, MSM_CAM_IOCTL_CTRL_COMMAND, &ctrlCmd) < 0)
        LOGE("ioctl CAMERA_EXIT fd %d error %s",
             mCameraControlFd, strerror(errno));

    LOGV("Stopping the conf thread");
    rc = pthread_join(mCamConfigThread, NULL);
    if (rc)
        LOGE("config_thread exit failure: %s", strerror(errno));
    else {
        LOGV("pthread_join on config_thread");
    }

    LOGV("Stopping the jpeg thread");
    rc = pthread_join(jpegThread, NULL);
    if (rc)
        LOGE("config_thread exit failure: %s", strerror(errno));
    else {
        LOGV("pthread_join on config_thread");
    }


    close(mCameraControlFd);
    mCameraControlFd = -1;

    LOGV("Stopping the w_thread");
    pthread_detach(w_thread);
    LOGV("Stopping the jpegThread");
    pthread_detach(jpegThread);
    LOGV("Killing the w_thread");
    pthread_kill(w_thread,9);
    LOGV("Killing the jpegThread");
    pthread_kill(jpegThread,9);


    close(mCameraControlFd);
    mCameraControlFd = -1;

#if DLOPEN_LIBMMCAMERA
    if (libmmcamera) {
        ::dlclose(libmmcamera);
        LOGV("dlclose(libmmcamera)");
        libmmcamera = NULL;
    }
    if (libmmcamera_target) {
        ::dlclose(libmmcamera_target);
        LOGV("dlclose(libmmcamera_target)");
        libmmcamera_target = NULL;
    }
#endif
    ::dlclose(libmmcamera);
    libmmcamera = NULL;
    ::dlclose(libmmcamera_target);
    libmmcamera_target = NULL;

    Mutex::Autolock lock(&singleton_lock);
    singleton_releasing = true;

    LOGV("Kalim Hardware Release ------- Empieza");
    // Mutex::Autolock lock(&singleton_lock);
    singleton.clear();
    singleton_releasing = false;
    singleton_wait.signal();
    LOGV("Kalim Hardware Release ------- FIN");

    //FIXME: crash when released
    LOGV("KALIM Join IMAGE");
    LINK_jpeg_encoder_join();


    LOGD("release X");
}

QualcommCameraHardware::~QualcommCameraHardware()
{
    LOGD("~QualcommCameraHardware E");
    Mutex::Autolock lock(&singleton_lock);
    singleton.clear();
    singleton_releasing = false;
    singleton_wait.signal();
    LOGD("~QualcommCameraHardware X");
}

sp<IMemoryHeap> QualcommCameraHardware::getRawHeap() const
{
    LOGV("getRawHeap");
    return mRawHeap != NULL ? mRawHeap->mHeap : NULL;
}

sp<IMemoryHeap> QualcommCameraHardware::getPreviewHeap() const
{
    LOGV("getPreviewHeap");
    return mPreviewHeap != NULL ? mPreviewHeap->mHeap : NULL;
}

status_t QualcommCameraHardware::startPreviewInternal()
{
    LOGV("startPreview E");

    if(mCameraRunning) {
        LOGV("startPreview X: preview already running.");
        return NO_ERROR;
    }

    if (!mPreviewInitialized) {
        mPreviewInitialized = initPreview();
        if (!mPreviewInitialized) {
            LOGE("startPreview X initPreview failed.  Not starting preview.");
            return UNKNOWN_ERROR;
        }
    }

/*
    int ret;
    unsigned long timeout = 2000; // 2s
    struct pollfd fds[] = {
        { mCameraControlFd, POLLIN | POLLRDNORM },
    };

    // polling, found in Donut dmesg.
    ret = poll(fds, 1, timeout);
    switch (ret) {
        case -1:
            LOGE("poll error");
            break;
        case 0:
            LOGE("poll timeout");
            break;
        default:
            if (fds[0].revents & POLLIN) {
                LOGV("frame is ready!");
            }
    }
 */

    mCameraRunning = native_start_preview(mCameraControlFd);
    if (!mCameraRunning) {
        deinitPreview();
        mPreviewInitialized = false;
        LOGE("startPreview X: native_start_preview failed!");
        return UNKNOWN_ERROR;
    }

    LOGV("startPreview X");
    return NO_ERROR;
}

status_t QualcommCameraHardware::startPreview()
{
    Mutex::Autolock l(&mLock);
    return startPreviewInternal();
}

void QualcommCameraHardware::stopPreviewInternal()
{
    LOGV("stopPreviewInternal E: %d", mCameraRunning);
    if (mCameraRunning) {
        // Cancel auto focus.
        if (mMsgEnabled & CAMERA_MSG_FOCUS) {
            LOGV("canceling autofocus");
            cancelAutoFocus();
        }

        LOGV("Stopping preview");
        mCameraRunning = !native_stop_preview(mCameraControlFd);
        if (!mCameraRunning && mPreviewInitialized) {
            deinitPreview();
            mPreviewInitialized = false;
        }
        else LOGE("stopPreviewInternal: failed to stop preview");
    }
    LOGV("stopPreviewInternal X: %d", mCameraRunning);
}

void QualcommCameraHardware::stopPreview()
{
    LOGV("stopPreview: E");
    Mutex::Autolock l(&mLock);

    if(mMsgEnabled & CAMERA_MSG_VIDEO_FRAME)
           return;

    stopPreviewInternal();

    LOGV("stopPreview: X");
}

void QualcommCameraHardware::runAutoFocus()
{
    mAutoFocusThreadLock.lock();
    mAutoFocusFd = open(MSM_CAMERA_CONTROL, O_RDWR);
    if (mAutoFocusFd < 0) {
        LOGE("autofocus: cannot open %s: %s",
             MSM_CAMERA_CONTROL,
             strerror(errno));
        mAutoFocusThreadRunning = false;
        mAutoFocusThreadLock.unlock();
        return;
    }

#if DLOPEN_LIBMMCAMERA
    // We need to maintain a reference to libmmcamera.so for the duration of the
    // AF thread, because we do not know when it will exit relative to the
    // lifetime of this object.  We do not want to dlclose() libmmcamera while
    // LINK_cam_frame is still running.
    void *libhandle = ::dlopen("libmmcamera.so", RTLD_NOW);
    LOGV("AF: loading libmmcamera at %p", libhandle);
    if (!libhandle) {
        LOGE("FATAL ERROR: could not dlopen libmmcamera.so: %s", dlerror());
        close(mAutoFocusFd);
        mAutoFocusFd = -1;
        mAutoFocusThreadRunning = false;
        mAutoFocusThreadLock.unlock();
        return;
    }
#endif

    /* This will block until either AF completes or is cancelled. */
    LOGV("af start (fd %d)", mAutoFocusFd);
    bool status = native_set_afmode(mAutoFocusFd, AF_MODE_AUTO);
    LOGV("af done: %d", (int)status);
    mAutoFocusThreadRunning = false;
    close(mAutoFocusFd);
    mAutoFocusFd = -1;
    mAutoFocusThreadLock.unlock();

    if (mMsgEnabled & CAMERA_MSG_FOCUS)
        mNotifyCb(CAMERA_MSG_FOCUS, status, 0, mCallbackCookie);

#if DLOPEN_LIBMMCAMERA
    if (libhandle) {
        ::dlclose(libhandle);
        LOGV("AF: dlclose(libmmcamera)");
    }
#endif
}

status_t QualcommCameraHardware::cancelAutoFocus()
{
    LOGV("cancelAutoFocus E");
    native_cancel_afmode(mCameraControlFd, mAutoFocusFd);
    LOGV("cancelAutoFocus X");

    /* Needed for eclair camera PAI */
    return NO_ERROR;
}

void *auto_focus_thread(void *user)
{
    LOGV("auto_focus_thread E");
    sp<QualcommCameraHardware> obj = QualcommCameraHardware::getInstance();
    if (obj != 0) {
        obj->runAutoFocus();
    }
    else LOGW("not starting autofocus: the object went away!");
    LOGV("auto_focus_thread X");
    return NULL;
}

status_t QualcommCameraHardware::autoFocus()
{
    LOGV("autoFocus E");
    Mutex::Autolock l(&mLock);

    if (mCameraControlFd < 0) {
        LOGE("not starting autofocus: main control fd %d", mCameraControlFd);
        return UNKNOWN_ERROR;
    }

    /* Not sure this is still needed with new APIs ..
    if (mMsgEnabled & CAMERA_MSG_FOCUS) {
        LOGW("Auto focus is already in progress");
        return NO_ERROR;
        // No idea how to rewrite this
        //return mAutoFocusCallback == af_cb ? NO_ERROR : INVALID_OPERATION;
    }*/

    {
        mAutoFocusThreadLock.lock();
        if (!mAutoFocusThreadRunning) {
            // Create a detatched thread here so that we don't have to wait
            // for it when we cancel AF.
            pthread_t thr;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            mAutoFocusThreadRunning =
                !pthread_create(&thr, &attr,
                                auto_focus_thread, NULL);
            if (!mAutoFocusThreadRunning) {
                LOGE("failed to start autofocus thread");
                mAutoFocusThreadLock.unlock();
                return UNKNOWN_ERROR;
            }
        }
        mAutoFocusThreadLock.unlock();
    }

    LOGV("autoFocus X");
    return NO_ERROR;
}

void QualcommCameraHardware::runSnapshotThread(void *data)
{
    LOGV("runSnapshotThread E");
    if (native_start_snapshot(mCameraControlFd))
        receiveRawPicture();
    else
        LOGE("main: native_start_snapshot failed!");

    mSnapshotThreadWaitLock.lock();
    mSnapshotThreadRunning = false;
    mSnapshotThreadWait.signal();
    mSnapshotThreadWaitLock.unlock();

    LOGV("runSnapshotThread X");
}

void *snapshot_thread(void *user)
{
    LOGD("snapshot_thread E");
    sp<QualcommCameraHardware> obj = QualcommCameraHardware::getInstance();
    if (obj != 0) {
        obj->runSnapshotThread(user);
    }
    else LOGW("not starting snapshot thread: the object went away!");
    LOGD("snapshot_thread X");
    return NULL;
}

status_t QualcommCameraHardware::takePicture()
{
    LOGV("takePicture: E");
    Mutex::Autolock l(&mLock);

    // Wait for old snapshot thread to complete.
    mSnapshotThreadWaitLock.lock();
    while (mSnapshotThreadRunning) {
        LOGV("takePicture: waiting for old snapshot thread to complete.");
        mSnapshotThreadWait.wait(mSnapshotThreadWaitLock);
        LOGV("takePicture: old snapshot thread completed.");
    }

    stopPreviewInternal();

    if (!initRaw(mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE)) { /* not sure if this is right */
        LOGE("initRaw failed. Not taking picture.");
        return UNKNOWN_ERROR;
    }

    mShutterLock.lock();
    mShutterPending = true;
    mShutterLock.unlock();

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    mSnapshotThreadRunning = !pthread_create(&mSnapshotThread,
                                             &attr,
                                             snapshot_thread,
                                             NULL);
    mSnapshotThreadWaitLock.unlock();

    LOGV("takePicture: X");
    return mSnapshotThreadRunning ? NO_ERROR : UNKNOWN_ERROR;
}

status_t QualcommCameraHardware::cancelPicture()
{
    LOGV("cancelPicture: EX");

    return NO_ERROR;
}

status_t QualcommCameraHardware::setParameters(
        const CameraParameters& params)
{
    LOGV("setParameters: E params = %p", &params);

    Mutex::Autolock l(&mLock);

    // Set preview size.
    preview_size_type *ps = preview_sizes;
    {
        int width, height;
        params.getPreviewSize(&width, &height);
        LOGV("requested size %d x %d", width, height);
        // Validate the preview size
        size_t i;
        for (i = 0; i < PREVIEW_SIZE_COUNT; ++i, ++ps) {
            if (width == ps->width && height == ps->height)
                break;
        }
        if (i == PREVIEW_SIZE_COUNT) {
            LOGE("Invalid preview size requested: %dx%d",
                 width, height);
            return BAD_VALUE;
        }
    }

    // hardcoded sizes got from Donut logcat
    mPreviewWidth = mDimension.display_width = 240; //ps->width;
    mPreviewHeight = mDimension.display_height = 160; //ps->height;

    LOGV("actual size %d x %d", mPreviewWidth, mPreviewHeight);

    // FIXME: validate snapshot sizes,
    params.getPictureSize(&mRawWidth, &mRawHeight);
    mDimension.picture_width = mRawWidth;
    mDimension.picture_height = mRawHeight;

    // Set up the jpeg-thumbnail-size parameters.
    {
        int val;

        val = params.getInt("jpeg-thumbnail-width");
        if (val < 0) {
            mDimension.ui_thumbnail_width= THUMBNAIL_WIDTH;
            LOGW("jpeg-thumbnail-width is not specified: defaulting to %d",
                 THUMBNAIL_WIDTH);
        }
        else mDimension.ui_thumbnail_width = val;

        val = params.getInt("jpeg-thumbnail-height");
        if (val < 0) {
            mDimension.ui_thumbnail_height= THUMBNAIL_HEIGHT;
            LOGW("jpeg-thumbnail-height is not specified: defaulting to %d",
                 THUMBNAIL_HEIGHT);
        }
        else mDimension.ui_thumbnail_height = val;
    }

    mParameters = params;

    setWhiteBalance();
    setEffect();
    setAntibanding();
    // FIXME: set nightshot and luma adaptatiom

    LOGV("setParameters: X");
    return NO_ERROR ;
}

CameraParameters QualcommCameraHardware::getParameters() const
{
    LOGV("getParameters: EX");
    return mParameters;
}

extern "C" sp<CameraHardwareInterface> openCameraHardware()
{
    LOGV("*-------------------------------------------------------------------------------------*");
    LOGV("[M a@openCameraHardware: call createInstance v: %s",version);
    LOGV("*-------------------------------------------------------------------------------------*");
    return QualcommCameraHardware::createInstance();
}

wp<QualcommCameraHardware> QualcommCameraHardware::singleton;

// If the hardware already exists, return a strong pointer to the current
// object. If not, create a new hardware object, put it in the singleton,
// and return it.
sp<CameraHardwareInterface> QualcommCameraHardware::createInstance()
{
    LOGD("createInstance: E");

    LOGV("get into singleton lock");
    Mutex::Autolock lock(&singleton_lock);

    // Wait until the previous release is done.
    while (singleton_releasing) {
        LOGD("Wait for previous release.");
        singleton_wait.wait(singleton_lock);
    }

    if (singleton != 0) {
        sp<CameraHardwareInterface> hardware = singleton.promote();
        if (hardware != 0) {
            LOGD("createInstance: X return existing hardware=%p", &(*hardware));
            return hardware;
        }
    }

    {
        struct stat st;
        int rc = stat("/dev/oncrpc", &st);
        if (rc < 0) {
            LOGD("createInstance: X failed to create hardware: %s", strerror(errno));
            return NULL;
        }
    }

    QualcommCameraHardware *cam = new QualcommCameraHardware();
    sp<QualcommCameraHardware> hardware(cam);
    singleton = hardware;

    cam->startCamera(); // mCameraControlFd evaluated in this function
    cam->initDefaultParameters(); // mCameraControlFd would be used to setParameters

    LOGD("createInstance: X created hardware=%p", &(*hardware));
    return hardware;
}

// For internal use only, hence the strong pointer to the derived type.
sp<QualcommCameraHardware> QualcommCameraHardware::getInstance()
{
    LOGV("getInstance: E");
    sp<CameraHardwareInterface> hardware = singleton.promote();
    if (hardware != 0) {
        LOGV("getInstance: X Search instance of hardware");
        return sp<QualcommCameraHardware>(static_cast<QualcommCameraHardware*>(hardware.get()));
    } else {
        LOGV("getInstance: X new instance of hardware");
        return sp<QualcommCameraHardware>();
    }
    LOGE("Fail getInstance: X");
}

void QualcommCameraHardware::receivePreviewFrame(struct msm_frame_t *frame)
{
    LOGV("receivePreviewFrame E");

    if (!mCameraRunning) {
        LOGE("ignoring preview callback--camera has been stopped");
        return;
    }

    // Find the offset within the heap of the current buffer.
    ssize_t offset =
        (ssize_t)frame->buffer - (ssize_t)mPreviewHeap->mHeap->base();
    offset /= mPreviewFrameSize;

    LOGV("offset: %lu\n", (unsigned long int)offset);

    mInPreviewCallback = true;
    if (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME)
        mDataCb(CAMERA_MSG_PREVIEW_FRAME, mPreviewHeap->mBuffers[offset], mCallbackCookie);

    if (mMsgEnabled & CAMERA_MSG_VIDEO_FRAME) {
        Mutex::Autolock rLock(&mRecordFrameLock);
        mDataCbTimestamp(systemTime(), CAMERA_MSG_VIDEO_FRAME, mPreviewHeap->mBuffers[offset], mCallbackCookie); /* guess? */
        //mDataCb(CAMERA_MSG_VIDEO_FRAME, mPreviewHeap->mBuffers[offset], mCallbackCookie);

        if (mReleasedRecordingFrame != true) {
            LOGV("block for release frame request/command");
            mRecordWait.wait(mRecordFrameLock);
        }
        mReleasedRecordingFrame = false;
    }

    mInPreviewCallback = false;

    LOGV("receivePreviewFrame X");
}

status_t QualcommCameraHardware::startRecording()
{
    LOGV("startRecording E");
    Mutex::Autolock l(&mLock);

    mReleasedRecordingFrame = false;
    mCameraRecording = true;

    return startPreviewInternal();
}

void QualcommCameraHardware::stopRecording()
{
    LOGV("stopRecording: E");
    Mutex::Autolock l(&mLock);

    {
        mRecordFrameLock.lock();
        mReleasedRecordingFrame = true;
        mRecordWait.signal();
        mRecordFrameLock.unlock();

        mCameraRecording = false;

        if(mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME) {
            LOGV("stopRecording: X, preview still in progress");
            return;
        }
    }

    stopPreviewInternal();
    LOGV("stopRecording: X");
}

void QualcommCameraHardware::releaseRecordingFrame(
       const sp<IMemory>& mem __attribute__((unused)))
{
    LOGV("releaseRecordingFrame E");
    Mutex::Autolock l(&mLock);
    Mutex::Autolock rLock(&mRecordFrameLock);
    mReleasedRecordingFrame = true;
    mRecordWait.signal();
    LOGV("releaseRecordingFrame X");
}

bool QualcommCameraHardware::recordingEnabled()
{
    return (mCameraRunning && mCameraRecording);
}

void QualcommCameraHardware::notifyShutter()
{
    mShutterLock.lock();
    if (mShutterPending && (mMsgEnabled & CAMERA_MSG_SHUTTER)) {
        mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mCallbackCookie);
        mShutterPending = false;
    }
    mShutterLock.unlock();
}

void QualcommCameraHardware::receiveRawPicture()
{
    LOGV("receiveRawPicture: E");

    notifyShutter();

    if (mMsgEnabled & CAMERA_MSG_RAW_IMAGE) {
        if(native_get_picture(mCameraControlFd, &mCrop) == false) {
            LOGE("getPicture failed!");
            return;
        }

        // By the time native_get_picture returns, picture is taken. Call
        mDataCb(CAMERA_MSG_RAW_IMAGE, mRawHeap->mBuffers[0], mCallbackCookie);
    }
    else LOGV("Raw-picture callback was canceled--skipping.");

    if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
        mJpegSize = mRawWidth * mRawHeight * 3 / 2;
        if (LINK_jpeg_encoder_init()) {
            if(native_jpeg_encode()) {
                LOGV("receiveRawPicture: X (success)");
                return;
            }
            LOGE("jpeg encoding failed");
        }
        else LOGE("receiveRawPicture X: jpeg_encoder_init failed.");
    }
    else LOGV("JPEG callback is NULL, not encoding image.");
    if( mRawInitialized ) {
        deinitRaw();
    }

    LOGV("receiveRawPicture: X");
}

void QualcommCameraHardware::receiveJpegPictureFragment(
    uint8_t *buff_ptr, uint32_t buff_size)
{
    uint32_t remaining = mJpegHeap->mHeap->virtualSize();
    remaining -= mJpegSize;
    uint8_t *base = (uint8_t *)mJpegHeap->mHeap->base();

    LOGV("receiveJpegPictureFragment size %d", buff_size);
    if (buff_size > remaining) {
        LOGE("receiveJpegPictureFragment: size %d exceeds what "
             "remains in JPEG heap (%d), truncating",
             buff_size,
             remaining);
        buff_size = remaining;
    }
    memcpy(base + mJpegSize, buff_ptr, buff_size);
    mJpegSize += buff_size;
}

void QualcommCameraHardware::receiveJpegPicture(void)
{
    LOGV("receiveJpegPicture: E image (%d uint8_ts out of %d)",
         mJpegSize, mJpegHeap->mBufferSize);

    LOGD("mJpegHeap->mFrameOffset %d", mJpegHeap->mFrameOffset ) ;

    int index = 0, rc;

    if (mMsgEnabled & CAMERA_MSG_COMPRESSED_IMAGE) {
        // The reason we do not allocate into mJpegHeap->mBuffers[offset] is
        // that the JPEG image's size will probably change from one snapshot
        // to the next, so we cannot reuse the MemoryBase object.
        sp<MemoryBase> buffer = new
        MemoryBase(mJpegHeap->mHeap,
                   index * mJpegHeap->mBufferSize +
                   mJpegHeap->mFrameOffset,
                   mJpegSize);

        mDataCb(CAMERA_MSG_COMPRESSED_IMAGE, buffer, mCallbackCookie);
        buffer = NULL;
    }
    else LOGV("JPEG callback was cancelled--not delivering image.");

    // Kalim FAKE to avoid join makes fail ----------------------
    // LOGV("KALIM Join IMAGE 2");
    // LINK_jpeg_encoder_join();
    // ----------------------------------------------------------

    if( mRawInitialized ) {
        deinitRaw();
    }

    LOGV("receiveJpegPicture: X callback done.");
}

bool QualcommCameraHardware::previewEnabled()
{
    Mutex::Autolock l(&mLock);
    return (mCameraRunning && (mMsgEnabled & CAMERA_MSG_PREVIEW_FRAME));
}

int QualcommCameraHardware::getParm(
    const char *parm_str, const struct str_map *const parm_map)
{
    // Check if the parameter exists.
    const char *str = mParameters.get(parm_str);
    if (str == NULL) return NOT_FOUND;

    // Look up the parameter value.
    return attr_lookup(parm_map, str);
}

void QualcommCameraHardware::setEffect()
{
    int32_t value = getParm("effect", effect);
    if (value != NOT_FOUND) {
        native_set_parm(CAMERA_SET_PARM_EFFECT, sizeof(value), (void *)&value);
    }
}

void QualcommCameraHardware::setWhiteBalance()
{
    int32_t value = getParm("whitebalance", whitebalance);
    if (value != NOT_FOUND) {
        native_set_parm(CAMERA_SET_PARM_WB, sizeof(value), (void *)&value);
    }
}

void QualcommCameraHardware::setAntibanding()
{
    camera_antibanding_type value =
        (camera_antibanding_type) getParm("antibanding", antibanding);
    native_set_parm(CAMERA_SET_PARM_ANTIBANDING, sizeof(value), (void *)&value);
}

QualcommCameraHardware::MemPool::MemPool(int buffer_size, int num_buffers,
                                         int frame_size,
                                         int frame_offset,
                                         const char *name) :
    mBufferSize(buffer_size),
    mNumBuffers(num_buffers),
    mFrameSize(frame_size),
    mFrameOffset(frame_offset),
    mBuffers(NULL), mName(name)
{
    // empty
}

void QualcommCameraHardware::MemPool::completeInitialization()
{
    // If we do not know how big the frame will be, we wait to allocate
    // the buffers describing the individual frames until we do know their
    // size.

    if (mFrameSize > 0) {
        mBuffers = new sp<MemoryBase>[mNumBuffers];
        for (int i = 0; i < mNumBuffers; i++) {
            mBuffers[i] = new
                MemoryBase(mHeap,
                           i * mBufferSize + mFrameOffset,
                           mFrameSize);
        }
    }
}

QualcommCameraHardware::AshmemPool::AshmemPool(int buffer_size, int num_buffers,
                                               int frame_size,
                                               int frame_offset,
                                               const char *name) :
    QualcommCameraHardware::MemPool(buffer_size,
                                    num_buffers,
                                    frame_size,
                                    frame_offset,
                                    name)
{
    LOGV("constructing MemPool %s backed by ashmem: "
         "%d frames @ %d uint8_ts, offset %d, "
         "buffer size %d",
         mName,
         num_buffers, frame_size, frame_offset, buffer_size);

    int page_mask = getpagesize() - 1;
    int ashmem_size = buffer_size * num_buffers;
    ashmem_size += page_mask;
    ashmem_size &= ~page_mask;

    mHeap = new MemoryHeapBase(ashmem_size);

    completeInitialization();
}

static bool register_buf(int camfd,
                         int size,
                         int pmempreviewfd,
                         uint32_t offset,
                         uint8_t *buf,
                         msm_pmem_t pmem_type,
                         bool active,
                         bool register_buffer = true);

QualcommCameraHardware::PmemPool::PmemPool(const char *pmem_pool,
                                           int camera_control_fd,
                                           msm_pmem_t pmem_type,
                                           int buffer_size,
                                           int num_buffers,
                                           int frame_size,
                                           int frame_offset,
                                           const char *name) :
    QualcommCameraHardware::MemPool(buffer_size,
                                    num_buffers,
                                    frame_size,
                                    frame_offset,
                                    name),
    mPmemType(pmem_type),
    mCameraControlFd(camera_control_fd)
{
    LOGV("constructing MemPool %s backed by pmem pool %s: "
         "%d frames @ %d bytes, offset %d, buffer size %d",
         mName,
         pmem_pool, num_buffers, frame_size, frame_offset,
         buffer_size);

    // Make a new mmap'ed heap that can be shared across processes.

    mAlignedSize = clp2(buffer_size * num_buffers);

    sp<MemoryHeapBase> masterHeap =
        new MemoryHeapBase(pmem_pool, mAlignedSize, 0);
    sp<MemoryHeapPmem> pmemHeap = new MemoryHeapPmem(masterHeap, 0);
    if (pmemHeap->getHeapID() >= 0) {
        pmemHeap->slap();
        masterHeap.clear();
        mHeap = pmemHeap;
        pmemHeap.clear();

        mFd = mHeap->getHeapID();
        if (::ioctl(mFd, PMEM_GET_SIZE, &mSize)) {
            LOGE("pmem pool %s ioctl(PMEM_GET_SIZE) error %s (%d)",
                 pmem_pool,
                 ::strerror(errno), errno);
            mHeap.clear();
            return;
        }

        LOGV("pmem pool %s ioctl(fd = %d, PMEM_GET_SIZE) is %ld",
             pmem_pool,
             mFd,
             mSize.len);

        // Register preview buffers with the camera drivers.
        for (int cnt = 0; cnt < num_buffers; ++cnt) {
            register_buf(mCameraControlFd,
                         buffer_size,
                         mHeap->getHeapID(),
                         buffer_size * cnt,
                         (uint8_t *)mHeap->base() + buffer_size * cnt,
                         pmem_type, true,
                         true);
        }

    }
    else LOGE("pmem pool %s error: could not create master heap!",
              pmem_pool);

     completeInitialization();
}

QualcommCameraHardware::PmemPool::~PmemPool()
{
    LOGV("%s: %s E", __FUNCTION__, mName);

    // Unregister preview buffers with the camera drivers.
    for (int cnt = 0; cnt < mNumBuffers; ++cnt) {
        register_buf(mCameraControlFd,
                     mBufferSize,
                     mHeap->getHeapID(),
                     mBufferSize * cnt,
                     (uint8_t *)mHeap->base() + mBufferSize * cnt,
                     mPmemType, true,
                     false /* unregister */);
    }

    LOGV("destroying PmemPool %s: ", mName);
    LOGV("%s: %s X", __FUNCTION__, mName);
}

QualcommCameraHardware::MemPool::~MemPool()
{
    LOGV("destroying MemPool %s", mName);
    if (mFrameSize > 0)
        delete [] mBuffers;
    mHeap.clear();
    LOGV("destroying MemPool %s completed", mName);
}

QualcommCameraHardware::PreviewPmemPool::PreviewPmemPool(
                        int control_fd,
                        int buffer_size,
                        int num_buffers,
                        int frame_size,
                        int frame_offset,
                        const char *name) :
    QualcommCameraHardware::PmemPool("/dev/pmem_adsp", control_fd, MSM_PMEM_OUTPUT2,
                                 buffer_size,
                                 num_buffers,
                                 frame_size,
                                 frame_offset,
                                 name)
{
    LOGV("QualcommCameraHardware::PreviewPmemPool::PreviewPmemPool");
    if (initialized()) {
        //NOTE : SOME PREVIEWPMEMPOOL SPECIFIC CODE MAY BE ADDED
    }
}

QualcommCameraHardware::PreviewPmemPool::~PreviewPmemPool()
{
    LOGV("destroying PreviewPmemPool");
    if (initialized()) {
        LOGV("releasing PreviewPmemPool memory %p from module %d",
             base, QDSP_MODULE_VFETASK);
    }
}

static bool register_buf(int camfd,
                         int size,
                         int pmempreviewfd,
                         uint32_t offset,
                         uint8_t *buf,
                         msm_pmem_t pmem_type,
                         bool active,
                         bool register_buffer)
{
    struct msm_pmem_info_t pmemBuf;

    pmemBuf.type     = pmem_type;
    pmemBuf.fd       = pmempreviewfd;
    pmemBuf.vaddr    = buf;
    pmemBuf.y_off    = 0;
    pmemBuf.active   = active;

    if (pmem_type == MSM_PMEM_RAW_MAINIMG)
        pmemBuf.cbcr_off = 0;
    else
        pmemBuf.cbcr_off = ((size * 2 / 3) + 1) & ~1;

    LOGV("register_buf: camfd = %d, reg = %d buffer = %p",
         camfd, !register_buffer, buf);
    if (ioctl(camfd,
              register_buffer ?
              MSM_CAM_IOCTL_REGISTER_PMEM :
              MSM_CAM_IOCTL_UNREGISTER_PMEM,
              &pmemBuf) < 0) {
        LOGE("register_buf: MSM_CAM_IOCTL_(UN)REGISTER_PMEM fd %d error %s",
             camfd,
             strerror(errno));
        return false;
    }
    return true;
}

status_t QualcommCameraHardware::MemPool::dump(int fd, const Vector<String16>& args) const
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;
    snprintf(buffer, 255, "QualcommCameraHardware::AshmemPool::dump\n");
    result.append(buffer);
    if (mName) {
        snprintf(buffer, 255, "mem pool name (%s)\n", mName);
        result.append(buffer);
    }
    if (mHeap != 0) {
        snprintf(buffer, 255, "heap base(%p), size(%d), flags(%d), device(%s)\n",
                 mHeap->getBase(), mHeap->getSize(),
                 mHeap->getFlags(), mHeap->getDevice());
        result.append(buffer);
    }
    snprintf(buffer, 255, "buffer size (%d), number of buffers (%d),"
             " frame size(%d), and frame offset(%d)\n",
             mBufferSize, mNumBuffers, mFrameSize, mFrameOffset);
    result.append(buffer);
    write(fd, result.string(), result.size());
    return NO_ERROR;
}

static void receive_camframe_callback(struct msm_frame_t *frame)
{
    LOGV("receive_camframe_callback E");
    sp<QualcommCameraHardware> obj = QualcommCameraHardware::getInstance();
    if (obj != 0) {
        LOGV("receive_camframe_callback Starting PreviewFrame");
        obj->receivePreviewFrame(frame);
    }
    LOGV("receive_camframe_callback X");
}

static void receive_jpeg_fragment_callback(uint8_t *buff_ptr, uint32_t buff_size)
{
    LOGV("receive_jpeg_fragment_callback E");
    sp<QualcommCameraHardware> obj = QualcommCameraHardware::getInstance();
    if (obj != 0) {
        obj->receiveJpegPictureFragment(buff_ptr, buff_size);
    }
    LOGV("receive_jpeg_fragment_callback X");
}

static void receive_jpeg_callback(jpeg_event_t status)
{
    LOGV("receive_jpeg_callback E (completion status %d)", status);
    if (status == JPEG_EVENT_DONE) {
        sp<QualcommCameraHardware> obj = QualcommCameraHardware::getInstance();
        if (obj != 0) {
            obj->receiveJpegPicture();
        }
    }
    LOGV("receive_jpeg_callback X");
}

status_t QualcommCameraHardware::sendCommand(int32_t command, int32_t arg1,
                                             int32_t arg2)
{
    LOGV("sendCommand: EX");
    return BAD_VALUE;
}

}; // namespace android
