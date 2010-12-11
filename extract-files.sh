#!/bin/sh

# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

mkdir proprietary
adb pull /system/etc/agps_rm proprietary
adb pull /system/bin/akm8973 proprietary
adb pull /system/etc/AudioFilter.csv proprietary
adb pull /system/etc/AudioPara4.csv proprietary
adb pull /system/etc/AudioPreProcess.csv proprietary
adb pull /system/usr/keylayout/AVRCP.kl proprietary
adb pull /system/usr/keylayout/bahamas-keypad.kl proprietary
adb pull /system/usr/keylayout/h2w_headset.kl proprietary
adb pull /system/usr/keylayout/qwerty.kl proprietary
adb pull /system/etc/firmware/brf6300.bin proprietary
adb pull /system/etc/firmware/brf6350.bin proprietary
adb pull /system/etc/wifi/Fw1251r1c.bin proprietary
adb pull /system/etc/wifi/tiwlan.ini proprietary
adb pull /system/lib/libA2DP.so proprietary
adb pull /system/lib/libaudioeq.so proprietary
adb pull /system/lib/egl/libGLES_qcom.so proprietary
adb pull /system/lib/libgps.so proprietary
adb pull /system/lib/libhtc_acoustic.so proprietary
adb pull /system/lib/libhtc_ril.so proprietary
adb pull /system/lib/libmm-adspsvc.so proprietary
adb pull /system/lib/liboemcamera.so proprietary
adb pull /system/lib/libOmxH264Dec.so proprietary
adb pull /system/lib/libOmxMpeg4Dec.so proprietary
adb pull /system/lib/libOmxVidEnc.so proprietary
adb pull /system/lib/libt9.so proprietary
adb pull /system/usr/keychars/qwerty.kcm.bin proprietary
adb pull /system/usr/keychars/qwerty2.kcm.bin proprietary
adb pull /system/lib/hw/sensors.bahamas.so proprietary
adb pull /system/etc/spn-conf.xml proprietary
adb pull /system/etc/voicemail-conf.xml proprietary
