# Copyright (C) 2009 The Android Open Source Project
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
# This file is the build configuration for a full Android
# build for tattoo hardware. This cleanly combines a set of
# device-specific aspects (drivers) with a device-agnostic
# product configuration (apps).
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)
$(call inherit-product, device/htc/tattoo/device_tattoo_eu.mk)

# Specific tattoo init.rc
#PRODUCT_COPY_FILES += \
#    device/htc/tattoo/init.rc:root/init.rc


PRODUCT_PROPERTY_OVERRIDES := \
    ro.media.dec.jpeg.memcap=10000000
    
PRODUCT_PROPERTY_OVERRIDES += \
    ro.ril.ecc.HTC-WWE=999 \
    ro.ril.ecc.HTC-Russia=01,02,03,04,001,002,003,004 \
    ro.ril.ecc.HTC-EastEurope=92,93,94 \
    ro.ril.enable.a52.HTC-ITA=1 \
    ro.ril.enable.a53.HTC-ITA=1

PRODUCT_PROPERTY_OVERRIDES += \
    rild.libpath=/system/lib/libhtc_ril.so \
    ro.ril.hep = 1 \
    ro.ril.enable.dtm = 1 \
    ro.ril.gprsclass = 12 \
    ro.ril.hsdpa.category = 8 \
    ro.ril.hsxpa = 1 \
    ro.ril.def.agps.mode = 2 \
    wifi.interface = tiwlan0 \
    ro.ril.htcmaskw1.bitmask = 4294967295 \
    ro.ril.htcmaskw1 = 14449

# Time between scans in seconds. Keep it high to minimize battery drain.
# This only affects the case in which there are remembered access points,
# but none are in range.
PRODUCT_PROPERTY_OVERRIDES += \
    wifi.supplicant_scan_interval=30
# density in DPI of the LCD of this board. This is used to scale the UI
# appropriately. If this property is not defined, the default value is 120 dpi. 
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=120
    
PRODUCT_PROPERTY_OVERRIDES += \
    keyguard.no_require_sim=true \
    ro.secure=0 \
    ro.tether.denied=true

# Disable tethering by default

# Disable JIT by default
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.execution-mode=int:fast
    
# The OpenGL ES API level, 1.0 here
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=65536

# Active stagefright
PRODUCT_PROPERTY_OVERRIDES += \
    media.stagefright.enable-http=true \
    media.stagefright.enable-player=true \
    media.stagefright.enable-meta=true \
    media.stagefright.enable-record=true \
    media.stagefright.enable-scan=true

PRODUCT_PROPERTY_OVERRIDES += \
    ro.com.google.locationfeatures=1

# View configuration for QVGA
PRODUCT_PROPERTY_OVERRIDES += \
    view.fading_edge_length=8 \
    view.touch_slop=15 \
    view.minimum_fling_velocity=25 \
    view.scroll_friction=0.008
    
# media configuration xml file
PRODUCT_COPY_FILES += \
    device/htc/tattoo/media_profiles.xml:/system/etc/media_profiles.xml
    
# stuff common to all HTC phones
$(call inherit-product, device/htc/common/common.mk)

$(call inherit-product, $(SRC_TARGET_DIR)/product/full.mk)

# Discard inherited values and use our own instead.
PRODUCT_NAME := full_tattoo
PRODUCT_DEVICE := tattoo
PRODUCT_MODEL := Full Android on Tattoo
