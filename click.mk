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

# Defines for Vold to config fstab defs
PRODUCT_COPY_FILES += \
    device/htc/click/vold.fstab:system/etc/vold.fstab

# Libs to be added to this ROM
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
    libcamera \
    librpc \
    dhcpcd.conf

#WiFi stuff
PRODUCT_PACKAGES += \
    wpa_supplicant.conf \
    tiwlan.ini \
    dhcpcd.conf \
    wlan_cu \
    wlan_loader \
    libCustomWifi

#HotSpot
PRODUCT_PACKAGES += \
    tiap_loader \
    tiap_cu \
    tiwlan_ap.ini \
    hostap \
    hostapd.conf

# Add Gallery 3D / Normal
PRODUCT_PACKAGES += Gallery

# Boot kernel files
PRODUCT_COPY_FILES += \
    device/htc/click/init.bahamas.rc:root/init.bahamas.rc \
    device/htc/click/ueventd.bahamas.rc:root/ueventd.bahamas.rc

# from device_dream_sapphire.mk
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml

# keychars and keylayout files
PRODUCT_COPY_FILES += \
    device/htc/click/custom/keychars/bahamas-keypad.kcm.bin:system/usr/keychars/bahamas-keypad.kcm.bin \
    device/htc/click/custom/keychars/qwerty.kcm.bin:system/usr/keychars/qwerty.kcm.bin \
    device/htc/click/custom/keychars/qwerty2.kcm.bin:system/usr/keychars/qwerty2.kcm.bin \
    device/htc/click/custom/keylayout/AVRCP.kl:system/usr/keylayout/AVRCP.kl \
    device/htc/click/custom/keylayout/bahamas-keypad.kl:system/usr/keylayout/bahamas-keypad.kl \
    device/htc/click/custom/keylayout/h2w_headset.kl:system/usr/keylayout/h2w_headset.kl \
    device/htc/click/custom/keylayout/qwerty.kl:system/usr/keylayout/qwerty.kl

# extra etc config files
PRODUCT_COPY_FILES += \
    device/htc/click/custom/sysctl.conf:system/etc/sysctl.conf

# precompiled files for /system/bin
PRODUCT_COPY_FILES += \
    device/htc/click/custom/calibrate_screen:system/bin/calibrate_screen \
    device/htc/click/custom/rzscontrol:system/bin/rzscontrol

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

# See comment at the top of this file. This is where the other
# half of the device-specific product definition file takes care
# of the aspects that require proprietary drivers that aren't
# commonly available
$(call inherit-product-if-exists, vendor/htc/click/click-vendor.mk)

# Added all the kernel modules to be copyed
$(call inherit-product-if-exists, device/htc/click/KernelModules.mk)

PRODUCT_LOCALES += ldpi,mdpi
