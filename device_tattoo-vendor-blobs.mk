# libgps is necessary to complete the compilation
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/logo.rle:root/logo.rle \
    device/htc/tattoo/proprietary/libgps.so:obj/lib/libgps.so

# proprietary for /system/squashfs
PRODUCT_COPY_FILES += \
	device/htc/tattoo/custom/modules.sqf:system/squashfs/modules.sqf

# Files in /system/etc
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/passwd:system/etc/passwd \
    device/htc/tattoo/custom/init.d/00sysctl:system/etc/init.d/00sysctl \
    device/htc/tattoo/custom/init.d/02squashfs:system/etc/init.d/02squashsf \
    device/htc/tattoo/custom/init.d/08compcache:system/etc/init.d/08compcache \
    device/htc/tattoo/custom/init.d/20opt_and_fix:system/etc/init.d/20opt_and_fix \
    device/htc/tattoo/custom/init.d/80userinit:system/etc/init.d/80userinit \
    device/htc/tattoo/custom/init.d/99complete:system/etc/init.d/99complete \
    device/htc/tattoo/custom/sysctl.conf:system/etc/sysctl.conf \
    device/htc/tattoo/custom/placeholder:system/lib/modules/placeholder

# Tattoo Calibration app
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/TattooCalibration.apk:system/app/TattooCalibration.apk

# proprietary for /system/sbin
PRODUCT_COPY_FILES += \
	device/htc/tattoo/custom/dropbear:system/xbin/dropbear \
	device/htc/tattoo/custom/dropbearkey:system/xbin/dropbearkey \
	device/htc/tattoo/custom/rzscontrol:system/xbin/rzscontrol

# proprietary for /system/usr/keychars and /system/usr/keylayout
PRODUCT_COPY_FILES += \
	device/htc/tattoo/proprietary/qwerty2.kcm.bin:system/usr/keychars/qwerty2.kcm.bin \
	device/htc/tattoo/proprietary/qwerty.kcm.bin:system/usr/keychars/qwerty.kcm.bin

# boot partition
PRODUCT_COPY_FILES += \
	device/htc/tattoo/custom/tattoo-hack.ko:root/sbin/tattoo-hack.ko \

# Other bin stuff
PRODUCT_COPY_FILES += \
	device/htc/tattoo/proprietary/akm8973:system/bin/akm8973

# proprietary stuff
PRODUCT_COPY_FILES += \
	device/htc/tattoo/proprietary/AudioPara4.csv:system/etc/AudioPara4.csv \
	device/htc/tattoo/proprietary/AudioFilter.csv:system/etc/AudioFilter.csv \
	device/htc/tattoo/proprietary/AudioPreProcess.csv:system/etc/AudioPreProcess.csv \
	device/htc/tattoo/proprietary/Fw1251r1c.bin:system/etc/wifi/Fw1251r1c.bin \
	device/htc/tattoo/proprietary/tiwlan.ini:system/etc/wifi/tiwlan.ini \
	device/htc/tattoo/proprietary/libA2DP.so:system/lib/libA2DP.so \
	device/htc/tattoo/proprietary/libaudioeq.so:system/lib/libaudioeq.so \
	device/htc/tattoo/proprietary/libGLES_qcom.so:system/lib/egl/libGLES_qcom.so \
	device/htc/tattoo/proprietary/libgps.so:system/lib/libgps.so \
	device/htc/tattoo/proprietary/liboemcamera.so:system/lib/liboemcamera.so \
	device/htc/tattoo/proprietary/libOmxH264Dec.so:system/lib/libOmxH264Dec.so \
	device/htc/tattoo/proprietary/libOmxMpeg4Dec.so:system/lib/libOmxMpeg4Dec.so \
	device/htc/tattoo/proprietary/libOmxVidEnc.so:system/lib/libOmxVidEnc.so \
	device/htc/tattoo/proprietary/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    	device/htc/tattoo/proprietary/libhtc_acoustic.so:system/lib/libhtc_acoustic.so \
    	device/htc/tattoo/proprietary/libhtc_ril.so:system/lib/libhtc_ril.so \
    	device/htc/tattoo/proprietary/brf6300.bin:system/etc/firmware/brf6300.bin \
    	device/htc/tattoo/proprietary/brf6350.bin:system/etc/firmware/brf6350.bin \
    	device/htc/tattoo/proprietary/bahamas-keypad.kl:system/usr/keylayout/bahamas-keypad.kl \
    	device/htc/tattoo/proprietary/h2w_headset.kl:system/usr/keylayout/h2w_headset.kl \
    	device/htc/tattoo/proprietary/qwerty.kl:system/usr/keylayout/qwerty.kl \
	device/htc/tattoo/proprietary/agps_rm:/system/etc/agps_rm \
	device/htc/tattoo/proprietary/libt9.so:system/lib/libt9.so \
	device/htc/tattoo/proprietary/voicemail-conf.xml:system/etc/voicemail-conf.xml \
	device/htc/tattoo/proprietary/spn-conf.xml:system/etc/spn-conf.xml \
	device/htc/tattoo/proprietary/sensors.bahamas.so:system/lib/hw/sensors.bahamas.so


