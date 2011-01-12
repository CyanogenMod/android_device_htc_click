# libgps is necessary to complete the compilation
PRODUCT_COPY_FILES += \
    device/htc/tattoo/proprietary/libgps.so:obj/lib/libgps.so \
    device/htc/tattoo/custom/logo.rle:root/logo.rle

# proprietary for modules
$(call inherit-product, device/htc/tattoo/KernelModules.mk)

PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/backuptool.sh:system/bin/backuptool.sh

# proprietary for etc
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/10calibrate_screen:system/etc/init.d/10calibrate_screen \
    device/htc/tattoo/custom/sysctl.conf:system/etc/sysctl.conf

# Tattoo Calibration apps
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/app/HTCCalibrate.apk:system/app/HTCCalibrate.apk

PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/app/FancyWidget.apk:system/app/FancyWidget.apk

# proprietary for /system/bin
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/compcache:system/bin/compcache \
    device/htc/tattoo/custom/rzscontrol:system/bin/rzscontrol

# proprietary for /system/usr/keychars and /system/usr/keylayout
PRODUCT_COPY_FILES += \
    device/htc/tattoo/keychars/qwerty2.kcm.bin:system/usr/keychars/qwerty2.kcm.bin \
    device/htc/tattoo/keychars/qwerty.kcm.bin:system/usr/keychars/qwerty.kcm.bin

# Other bin stuff
PRODUCT_COPY_FILES += \
    device/htc/tattoo/proprietary/akm8973:system/bin/akm8973

# proprietary stuff
PRODUCT_COPY_FILES += \
    device/htc/tattoo/proprietary/AudioPara4.csv:system/etc/AudioPara4.csv \
    device/htc/tattoo/proprietary/AudioFilter.csv:system/etc/AudioFilter.csv \
    device/htc/tattoo/proprietary/AudioPreProcess.csv:system/etc/AudioPreProcess.csv \
    device/htc/tattoo/proprietary/fakecam/liboemcamera.so:system/lib/liboemcamera.so \
    device/htc/tattoo/proprietary/libmmcamera.so:system/lib/libmmcamera.so \
    device/htc/tattoo/proprietary/libmm-qcamera-tgt.so:system/lib/libmm-qcamera-tgt.so \
    device/htc/tattoo/proprietary/libmmjpeg.so:system/lib/libmmjpeg.so \
    device/htc/tattoo/proprietary/libaudioeq.so:system/lib/libaudioeq.so \
    device/htc/tattoo/proprietary/libqcamera.so:system/lib/libqcamera.so \
    device/htc/tattoo/proprietary/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    device/htc/tattoo/proprietary/libGLES_qcom.so:system/lib/egl/libGLES_qcom.so \
    device/htc/tattoo/proprietary/libgps.so:system/lib/libgps.so \
    device/htc/tattoo/proprietary/libOmxH264Dec.so:system/lib/libOmxH264Dec.so \
    device/htc/tattoo/proprietary/libOmxMpeg4Dec.so:system/lib/libOmxMpeg4Dec.so \
    device/htc/tattoo/proprietary/libOmxVidEnc.so:system/lib/libOmxVidEnc.so \
    device/htc/tattoo/proprietary/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    device/htc/tattoo/proprietary/libhtc_acoustic.so:system/lib/libhtc_acoustic.so \
    device/htc/tattoo/proprietary/libhtc_ril.so:system/lib/libhtc_ril.so \
    device/htc/tattoo/keylayout/bahamas-keypad.kl:system/usr/keylayout/bahamas-keypad.kl \
    device/htc/tattoo/keylayout/h2w_headset.kl:system/usr/keylayout/h2w_headset.kl \
    device/htc/tattoo/keylayout/qwerty.kl:system/usr/keylayout/qwerty.kl \
    device/htc/tattoo/firmware/Fw1251r1c.bin:system/etc/wifi/Fw1251r1c.bin \
    device/htc/tattoo/firmware/brf6300.bin:system/etc/firmware/brf6300.bin \
    device/htc/tattoo/firmware/brf6350.bin:system/etc/firmware/brf6350.bin \
    device/htc/tattoo/firmware/tiinit_5.3.53.bts:system/etc/firmware/tiinit_5.3.53.bts

ifdef WITH_WINDOWS_MEDIA
PRODUCT_COPY_FILES += \
    device/htc/tattoo/proprietary/libomx_wmadec_sharedlibrary.so:system/lib/libomx_wmadec_sharedlibrary.so \
    device/htc/tattoo/proprietary/libomx_wmvdec_sharedlibrary.so:system/lib/libomx_wmvdec_sharedlibrary.so \
    device/htc/tattoo/proprietary/libpvasfcommon.so:system/lib/libpvasfcommon.so \
    device/htc/tattoo/proprietary/libpvasflocalpbreg.so:system/lib/libpvasflocalpbreg.so \
    device/htc/tattoo/proprietary/libpvasflocalpb.so:system/lib/libpvasflocalpb.so \
    device/htc/tattoo/proprietary/pvasflocal.cfg:system/etc/pvasflocal.cfg
endif
