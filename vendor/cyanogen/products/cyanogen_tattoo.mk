$(call inherit-product, device/htc/tattoo/tattoo.mk)

# Inherit some common cyanogenmod stuff.
$(call inherit-product, vendor/cyanogen/products/common.mk)

# Include GSM stuff
$(call inherit-product, vendor/cyanogen/products/gsm.mk)

# Setup device specific product configuration.
PRODUCT_NAME := cyanogen_tattoo
PRODUCT_BRAND := htc
PRODUCT_DEVICE := tattoo
PRODUCT_MODEL := HTC Tattoo
PRODUCT_MANUFACTURER := HTC
PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=htc_tattoo BUILD_ID=FRG83 BUILD_DISPLAY_ID=GRH78 BUILD_FINGERPRINT=google/passion/passion/mahimahi:2.2.1/FRG83/60505:user/release-keys PRIVATE_BUILD_DESC="passion-user 2.2.1 FRG83 60505 release-keys"

# Enable Windows Media
WITH_WINDOWS_MEDIA := true

# Extra Tattoo overlay
PRODUCT_PACKAGE_OVERLAYS += vendor/cyanogen/overlay/tattoo

# Set ro.modversion
PRODUCT_PROPERTY_OVERRIDES += \
    ro.modversion=Kalim-10.0

# Copy tattoo specific prebuilt files
PRODUCT_COPY_FILES +=  \
    device/htc/tattoo/custom/bootanimation.zip:system/media/bootanimation.zip
