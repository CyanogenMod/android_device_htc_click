# config.mk
#
# Product-specific compile-time definitions.
#

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).
USE_CAMERA_STUB := true

# inherit from the proprietary version
-include device/htc/tattoo/BoardConfigVendor.mk

TARGET_BOARD_PLATFORM := msm7k
TARGET_ARCH_VARIANT := armv6j

TARGET_CPU_ABI := armeabi

TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true
TARGET_NO_RECOVERY := true

# Wifi related defines
BOARD_WPA_SUPPLICANT_DRIVER := CUSTOM
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := libWifiApi
BOARD_WLAN_TI_STA_DK_ROOT   := system/wlan/ti/sta_dk_4_0_4_32
WIFI_DRIVER_MODULE_PATH     := "/system/lib/modules/wlan.ko"
WIFI_DRIVER_MODULE_ARG      := ""
WIFI_DRIVER_MODULE_NAME     := "wlan"
WIFI_FIRMWARE_LOADER        := "wlan_loader"

TARGET_BOOTLOADER_BOARD_NAME := bahamas

TARGET_BOOTLOADER_LIBS := \
	libboot_board_dream_sapphire_combined \
	libboot_arch_msm7k \
	libboot_arch_armv6

TARGET_BOOTLOADER_LINK_SCRIPT := \
	hardware/msm7k/boot/boot.ld

TARGET_PROVIDES_INIT_RC := true

BOARD_KERNEL_CMDLINE := no_console_suspend=1 console=null
BOARD_KERNEL_BASE := 0x02E00000

BOARD_HAVE_BLUETOOTH := true

BOARD_HAS_LIMITED_EGL:=true

BOARD_VENDOR_USE_AKMD := akm8973

BOARD_VENDOR_QCOM_AMSS_VERSION := 1355

TARGET_HARDWARE_3D := false

BOARD_GPS_LIBRARIES := libgps librpc

# OpenGL drivers config file path
BOARD_EGL_CFG := device/htc/tattoo/egl.cfg


# Use libcamera2
BOARD_USES_OLD_CAMERA_HACK := true

# No authoring clock for OpenCore on DS
BOARD_NO_PV_AUTHORING_CLOCK := true

BOARD_USES_QCOM_LIBS := true

TARGET_RELEASETOOLS_EXTENSIONS := device/htc/common


TARGET_OTA_ASSERT_DEVICE := bahamas
PRODUCT_BUILD_PROP_OVERRIDES += TARGET_BOOTLOADER_BOARD_NAME=bahamas

# # cat /proc/mtd
# dev:    size   erasesize  name
# mtd0: 00040000 00020000 "misc"
# mtd1: 00500000 00020000 "recovery"
# mtd2: 00280000 00020000 "boot"
# mtd3: 05a00000 00020000 "system"
# mtd4: 05000000 00020000 "cache"
# mtd5: 127c0000 00020000 "userdata"
# Changed for Tattoo
BOARD_BOOTIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x00280000)
BOARD_RECOVERYIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x00500000)
BOARD_SYSTEMIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x09600000)
BOARD_USERDATAIMAGE_MAX_SIZE := $(call image-size-from-data-size,0x0a520000)

# The size of a block that can be marked bad.
BOARD_FLASH_BLOCK_SIZE := 153600

# Stagefright fully enabled
BUILD_WITH_FULL_STAGEFRIGHT := true


# JIT built in, but disabled by default
WITH_JIT := true
ENABLE_JSC_JIT := true

# Stop compiling test_* binaries for eng tag
#STOP_TEST_BINS := true

