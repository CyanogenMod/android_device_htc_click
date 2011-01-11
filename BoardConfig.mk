# config.mk
#
# Product-specific compile-time definitions.
#

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).
USE_CAMERA_STUB := true

# Fake building with froyo cam, as old libcam is not here yet
BOARD_USE_FROYO_LIBCAMERA := true

# inherit from the proprietary version
-include device/htc/tattoo/BoardConfigVendor.mk

# ARMv6-compatible processor rev 5 (v6l)
TARGET_BOARD_PLATFORM := msm7k
TARGET_ARCH_VARIANT := armv6j
TARGET_CPU_ABI := armeabi-v6j
TARGET_CPU_ABI2 := armeabi

TARGET_BOOTLOADER_BOARD_NAME := bahamas

TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true
TARGET_NO_RECOVERY := false

BOARD_LDPI_RECOVERY := true

TARGET_PREBUILT_RECOVERY_KERNEL := device/htc/tattoo/custom/recovery_kernel
TARGET_PREBUILT_KERNEL := device/htc/tattoo/custom/kernel

# Wifi related defines
BOARD_WPA_SUPPLICANT_DRIVER := CUSTOM
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := libWifiApi
BOARD_WLAN_TI_STA_DK_ROOT   := system/wlan/ti/sta_dk_4_0_4_32
WIFI_DRIVER_MODULE_PATH     := "/system/lib/modules/wlan.ko"
WIFI_DRIVER_MODULE_ARG      := ""
WIFI_DRIVER_MODULE_NAME     := "wlan"
WIFI_FIRMWARE_LOADER        := "wlan_loader"
WIFI_DRIVER_FW_STA_PATH     := "/etc/firmware/tiinit_5.3.53.bts"
WIFI_DRIVER_FW_AP_PATH      := "/etc/wifi/Fw1251r1c.bin"

TARGET_PROVIDES_INIT_RC := false

BOARD_KERNEL_CMDLINE := no_console_suspend=1 console=null
BOARD_KERNEL_BASE := 0x02E00000

BOARD_USES_GENERIC_AUDIO := false

# Use HTC USB Function Switch to enable tethering via USB
BOARD_USE_HTC_USB_FUNCTION_SWITCH := true

BOARD_USE_USB_MASS_STORAGE_SWITCH := true

BOARD_HAVE_BLUETOOTH := true

BOARD_VENDOR_USE_AKMD := akm8973

BOARD_VENDOR_QCOM_AMSS_VERSION := 1355

BOARD_HAS_LIMITED_EGL := true

TARGET_HARDWARE_3D := false

# OpenGL drivers config file path
BOARD_EGL_CFG := device/htc/tattoo/egl.cfg

# No authoring clock for OpenCore
# BOARD_NO_PV_AUTHORING_CLOCK := true

BOARD_USES_QCOM_LIBS := true

BOARD_USES_GPSSHIM := true

BOARD_GPS_NEEDS_XTRA := true

BOARD_GPS_LIBRARIES := libgps librpc

TARGET_LIBAGL_USE_GRALLOC_COPYBITS := true

BOARD_NO_RGBX_8888 := true

JS_ENGINE := v8

TARGET_RELEASETOOLS_EXTENSIONS := device/htc/common

# # cat /proc/mtd
# dev:    size   erasesize  name
# mtd0: 000a0000 00020000 "misc"
# mtd1: 00500000 00020000 "recovery"
# mtd2: 00280000 00020000 "boot"
# mtd3: 09600000 00020000 "system"
# mtd4: 09600000 00020000 "cache"
# mtd5: 0a520000 00020000 "userdata"
# Changed for Tattoo
BOARD_BOOTIMAGE_PARTITION_SIZE := 0x00280000
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 0x00500000
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 0x09600000
BOARD_USERDATAIMAGE_PARTITION_SIZE := 0x0a520000
BOARD_FLASH_BLOCK_SIZE := 131072
