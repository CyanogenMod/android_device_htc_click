    mkdir proprietary
    mkdir proprietary/app
    mkdir proprietary/keylayout
    mkdir proprietary/firmware
    mkdir proprietary/keychars
    adb pull /system/app/HTCCalibrate.apk proprietary/app/
    adb pull /system/etc/AudioPara4.csv proprietary/
    adb pull /system/etc/AudioFilter.csv proprietary/
    adb pull /system/etc/AudioPreProcess.csv proprietary/
    adb pull /system/lib/liboemcamera.so proprietary/
    adb pull /system/lib/libmmcamera.so proprietary/
    adb pull /system/lib/libmm-qcamera-tgt.so proprietary/
    adb pull /system/lib/libmmjpeg.so proprietary/
    adb pull /system/lib/libaudioeq.so proprietary/
    adb pull /system/lib/libqcamera.so proprietary/
    adb pull /system/lib/libmm-adspsvc.so proprietary/
    adb pull /system/lib/egl/libGLES_qcom.so proprietary/
    adb pull /system/lib/libgps.so proprietary/
    adb pull /system/lib/libOmxH264Dec.so proprietary/
    adb pull /system/lib/libOmxMpeg4Dec.so proprietary/
    adb pull /system/lib/libOmxVidEnc.so proprietary/
    adb pull /system/lib/libmm-adspsvc.so proprietary/
    adb pull /system/lib/libhtc_acoustic.so proprietary/
    adb pull /system/lib/libhtc_ril.so proprietary/
    adb pull /system/lib/libomx_wmadec_sharedlibrary.so proprietary/
    adb pull /system/lib/libomx_wmvdec_sharedlibrary.so proprietary/
    adb pull /system/lib/libpvasfcommon.so proprietary/
    adb pull /system/lib/libpvasflocalpbreg.so proprietary/
    adb pull /system/lib/libpvasflocalpb.so proprietary/
    adb pull /system/etc/pvasflocal.cfg proprietary/
    adb pull /system/bin/akm8973 proprietary/
    adb pull /system/usr/keychars/bahamas-keypad.kcm.bin proprietary/keychars/
    adb pull /system/usr/keychars/qwerty.kcm.bin proprietary/keychars/
    adb pull /system/usr/keychars/qwerty2.kcm.bin proprietary/keychars/
    adb pull /system/usr/keylayout/AVRCP.kl proprietary/keylayout/
    adb pull /system/usr/keylayout/bahamas-keypad.kl proprietary/keylayout/
    adb pull /system/usr/keylayout/h2w_headset.kl proprietary/keylayout/
    adb pull /system/usr/keylayout/qwerty.kl proprietary/keylayout/
    adb pull /system/etc/wifi/Fw1251r1c.bin proprietary/firmware/
    adb pull /system/etc/firmware/brf6300.bin proprietary/firmware/
    adb pull /system/etc/firmware/brf6350.bin proprietary/firmware/
