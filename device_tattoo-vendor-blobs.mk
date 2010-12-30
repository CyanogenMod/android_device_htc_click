# libgps is necessary to complete the compilation
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/logo.rle:root/logo.rle \
    device/htc/tattoo/proprietary/libgps.so:obj/lib/libgps.so

# Files in /system/etc
PRODUCT_COPY_FILES += \
    device/htc/tattoo/custom/passwd:system/etc/passwd \
    device/htc/tattoo/custom/sysctl.conf:system/etc/sysctl.conf \
    device/htc/tattoo/custom/permissions/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    device/htc/tattoo/custom/placeholder:system/lib/modules/placeholder \
    device/htc/tattoo/custom/modules/ip6_tunnel.ko:system/lib/modules/ip6_tunnel.ko \
    device/htc/tattoo/custom/modules/ipv6.ko:system/lib/modules/ipv6.ko \
    device/htc/tattoo/custom/modules/mip6.ko:system/lib/modules/mip6.ko \
    device/htc/tattoo/custom/modules/ramzswap.ko:system/lib/modules/ramzswap.ko \
    device/htc/tattoo/custom/modules/sit.ko:system/lib/modules/sit.ko \
    device/htc/tattoo/custom/modules/tunnel4.ko:system/lib/modules/tunnel4.ko \
    device/htc/tattoo/custom/modules/tunnel6.ko:system/lib/modules/tunnel6.ko \
    device/htc/tattoo/custom/modules/wlan.ko:system/lib/modules/wlan.ko \
    device/htc/tattoo/custom/modules/xfrm6_mode_beet.ko:system/lib/modules/xfrm6_mode_beet.ko \
    device/htc/tattoo/custom/modules/xfrm6_mode_transport.ko:system/lib/modules/xfrm6_mode_transport.ko \
    device/htc/tattoo/custom/modules/xfrm6_mode_tunnel.ko:system/lib/modules/xfrm6_mode_tunnel.ko \
    device/htc/tattoo/custom/modules/xt_TCPMSS.ko:system/lib/modules/xt_TCPMSS.ko \
    device/htc/tattoo/custom/modules/tattoo-hack.ko:system/lib/modules/tattoo-hack.ko

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
	device/htc/tattoo/custom/modules/tattoo-hack.ko:root/lib/modules/tattoo-hack.ko \

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
	device/htc/tattoo/proprietary/libmmcamera.so:system/lib/libmmcamera.so \
	device/htc/tattoo/proprietary/libmm-qcamera-tgt.so:system/lib/libmm-qcamera-tgt.so \
	device/htc/tattoo/proprietary/libqcamera.so:system/lib/libqcamera.so \
	device/htc/tattoo/proprietary/libmmjpeg.so:system/lib/libmmjpeg.so \
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

ifdef WITH_WINDOWS_MEDIA
PRODUCT_COPY_FILES += \
    device/htc/tattoo/proprietary/libomx_wmadec_sharedlibrary.so:system/lib/libomx_wmadec_sharedlibrary.so \
    device/htc/tattoo/proprietary/libomx_wmvdec_sharedlibrary.so:system/lib/libomx_wmvdec_sharedlibrary.so \
    device/htc/tattoo/proprietary/libpvasfcommon.so:system/lib/libpvasfcommon.so \
    device/htc/tattoo/proprietary/libpvasflocalpbreg.so:system/lib/libpvasflocalpbreg.so \
    device/htc/tattoo/proprietary/libpvasflocalpb.so:system/lib/libpvasflocalpb.so \
    device/htc/tattoo/proprietary/pvasflocal.cfg:system/etc/pvasflocal.cfg
endif

