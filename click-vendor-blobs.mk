# libgps is necessary to complete the compilation
PRODUCT_COPY_FILES += \
    device/htc/click/proprietary/libgps.so:obj/lib/libgps.so \
    device/htc/click/custom/logo.rle:root/logo.rle

# proprietary for modules
$(call inherit-product, device/htc/click/KernelModules.mk)

PRODUCT_COPY_FILES += \
    device/htc/click/custom/backuptool.sh:system/bin/backuptool.sh

# proprietary for etc
PRODUCT_COPY_FILES += \
    device/htc/click/custom/10calibrate_screen:system/etc/init.d/10calibrate_screen \
    device/htc/click/custom/sysctl.conf:system/etc/sysctl.conf

# click Calibration and other apps
PRODUCT_COPY_FILES += \
    device/htc/click/proprietary/app/HTCCalibrate.apk:system/app/HTCCalibrate.apk

# proprietary for /system/bin
PRODUCT_COPY_FILES += \
    device/htc/click/custom/compcache:system/bin/compcache \
    device/htc/click/custom/rzscontrol:system/bin/rzscontrol

# Other bin stuff
PRODUCT_COPY_FILES += \
    device/htc/click/proprietary/akm8973:system/bin/akm8973

# psfreedom files
PRODUCT_COPY_FILES += \
    device/htc/click/custom/modules.recovery/psfreedom.ko:recovery/root/res/modules/psfreedom.ko

# proprietary stuff
PRODUCT_COPY_FILES += \
    device/htc/click/proprietary/AudioPara4.csv:system/etc/AudioPara4.csv \
    device/htc/click/proprietary/AudioFilter.csv:system/etc/AudioFilter.csv \
    device/htc/click/proprietary/AudioPreProcess.csv:system/etc/AudioPreProcess.csv \
    device/htc/click/proprietary/liboemcamera.so:system/lib/liboemcamera.so \
    device/htc/click/proprietary/libmmcamera.so:system/lib/libmmcamera.so \
    device/htc/click/proprietary/libmm-qcamera-tgt.so:system/lib/libmm-qcamera-tgt.so \
    device/htc/click/proprietary/libmmjpeg.so:system/lib/libmmjpeg.so \
    device/htc/click/proprietary/libaudioeq.so:system/lib/libaudioeq.so \
    device/htc/click/proprietary/libqcamera.so:system/lib/libqcamera.so \
    device/htc/click/proprietary/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    device/htc/click/proprietary/libGLES_qcom.so:system/lib/egl/libGLES_qcom.so \
    device/htc/click/proprietary/libOmxH264Dec.so:system/lib/libOmxH264Dec.so \
    device/htc/click/proprietary/libOmxMpeg4Dec.so:system/lib/libOmxMpeg4Dec.so \
    device/htc/click/proprietary/libOmxVidEnc.so:system/lib/libOmxVidEnc.so \
    device/htc/click/proprietary/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    device/htc/click/proprietary/libhtc_acoustic.so:system/lib/libhtc_acoustic.so \
    device/htc/click/proprietary/libhtc_ril.so:system/lib/libhtc_ril.so \
    device/htc/click/firmware/Fw1251r1c.bin:system/etc/wifi/Fw1251r1c.bin \
    device/htc/click/firmware/brf6300.bin:system/etc/firmware/brf6300.bin \
    device/htc/click/firmware/brf6350.bin:system/etc/firmware/brf6350.bin

ifdef WITH_WINDOWS_MEDIA
PRODUCT_COPY_FILES += \
    device/htc/click/proprietary/libomx_wmadec_sharedlibrary.so:system/lib/libomx_wmadec_sharedlibrary.so \
    device/htc/click/proprietary/libomx_wmvdec_sharedlibrary.so:system/lib/libomx_wmvdec_sharedlibrary.so \
    device/htc/click/proprietary/libpvasfcommon.so:system/lib/libpvasfcommon.so \
    device/htc/click/proprietary/libpvasflocalpbreg.so:system/lib/libpvasflocalpbreg.so \
    device/htc/click/proprietary/libpvasflocalpb.so:system/lib/libpvasflocalpb.so \
    device/htc/click/proprietary/pvasflocal.cfg:system/etc/pvasflocal.cfg
endif
