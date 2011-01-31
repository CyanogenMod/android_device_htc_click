#
# Copyright (C) 2008 The Android Open Source Project
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
#

DEVICE_PACKAGE_OVERLAYS += device/htc/click/overlay

# Live wallpaper packages
PRODUCT_PACKAGES += \
    LiveWallpapersPicker \
    librs_jni

# Publish that we support the live wallpaper feature.
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:/system/etc/permissions/android.software.live_wallpaper.xml \
    device/htc/click/vold.fstab:system/etc/vold.fstab

$(call inherit-product, device/htc/click/click-vendor-blobs.mk)

PRODUCT_PACKAGES += \
    librs_jni \
    sensors.bahamas \
    lights.bahamas \
    copybit.bahamas \
    gralloc.bahamas \
    gps.bahamas \
    libOmxCore \
    libmm-omxcore \
    libOmxVidEnc \
    wlan_loader \
    tiwlan.ini \
    libcamera \
    librpc \
    dhcpcd.conf

PRODUCT_COPY_FILES += \
    device/htc/click/init.bahamas.rc:root/init.bahamas.rc \
    device/htc/click/ueventd.bahamas.rc:root/ueventd.bahamas.rc

# Add normal Gallery
PRODUCT_PACKAGES += Gallery

# from device_dream_sapphire.mk
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml

# keychars and keylayout files
PRODUCT_COPY_FILES += \
    device/htc/click/proprietary/keychars/bahamas-keypad.kcm.bin:system/usr/keychars/bahamas-keypad.kcm.bin \
    device/htc/click/proprietary/keychars/qwerty.kcm.bin:system/usr/keychars/qwerty.kcm.bin \
    device/htc/click/proprietary/keychars/qwerty2.kcm.bin:system/usr/keychars/qwerty2.kcm.bin \
    device/htc/click/proprietary/keylayout/AVRCP.kl:system/usr/keylayout/AVRCP.kl \
    device/htc/click/proprietary/keylayout/bahamas-keypad.kl:system/usr/keylayout/bahamas-keypad.kl \
    device/htc/click/proprietary/keylayout/h2w_headset.kl:system/usr/keylayout/h2w_headset.kl \
    device/htc/click/proprietary/keylayout/qwerty.kl:system/usr/keylayout/qwerty.kl

PRODUCT_PROPERTY_OVERRIDES += \
    ro.media.dec.jpeg.memcap=10000000

PRODUCT_PROPERTY_OVERRIDES += \
    rild.libpath=/system/lib/libhtc_ril.so \
    ro.ril.ecc.HTC-ELL=92,93,94 \
    ro.ril.ecc.HTC-WWE=999 \
    ro.ril.enable.a52.HTC-ITA=1 \
    ro.ril.enable.a53.HTC-ITA=1 \
    ro.ril.enable.a52=0 \
    ro.ril.enable.a53=1 \
    ro.ril.enable.dtm = 1 \
    ro.ril.gprsclass = 12 \
    ro.ril.hsdpa.category=8 \
    ro.ril.hsupa.category=5 \
    ro.ril.hsxpa=2 \
    ro.ril.def.agps.mode = 2

# Time between scans in seconds. Keep it high to minimize battery drain.
# This only affects the case in which there are remembered access points,
# but none are in range.
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.interface = tiwlan0 \
    wifi.supplicant_scan_interval=45 \
    ro.tether.denied=true

# density in DPI of the LCD of this board. This is used to scale the UI
# appropriately. If this property is not defined, the default value is 120 dpi. 
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=120

# View configuration for QVGA
PRODUCT_PROPERTY_OVERRIDES += \
    view.fading_edge_length=8 \
    view.touch_slop=15 \
    view.minimum_fling_velocity=25 \
    view.scroll_friction=0.008

# Default network type
# 0 => WCDMA Preferred.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.telephony.default_network=0 \
    ro.com.google.locationfeatures=1

PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.hw=1 \
    ro.opengles.version=65537 \
    persist.sys.use_dithering=0

# Disable fs check on boot by default
PRODUCT_PROPERTY_OVERRIDES += \
    sys.checkfs.fat=false

PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.notification_sound=tweeters.ogg \
    ro.config.alarm_alert=Alarm_Classic.ogg

# Enable JIT by default
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.execution-mode=int:fast

# VM heap size
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapsize=24m

# Enable compcache
PRODUCT_PROPERTY_OVERRIDES += \
    ro.compcache.default=18

# Makes HOME ADW / OTHERS to be always in memoory
PRODUCT_PROPERTY_OVERRIDES += \
    pref_lock_home=1

# media configuration xml file
PRODUCT_COPY_FILES += \
    device/htc/click/media_profiles.xml:/system/etc/media_profiles.xml

# Kernel Targets
ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := device/htc/click/custom/kernel
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

$(call inherit-product, device/common/gps/gps_eu_supl.mk)
$(call inherit-product, device/htc/common/common.mk)
$(call inherit-product, build/target/product/full_base.mk)

PRODUCT_NAME := htc_click
PRODUCT_DEVICE := click
