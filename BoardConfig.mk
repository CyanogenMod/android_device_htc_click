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
-include vendor/htc/click/BoardConfigVendor.mk

# ARMv6-compatible processor rev 5 (v6l)
TARGET_BOARD_PLATFORM := msm7k
TARGET_ARCH_VARIANT := armv6j
TARGET_CPU_ABI := armeabi-v6j
TARGET_CPU_ABI2 := armeabi

TARGET_BOOTLOADER_BOARD_NAME := bahamas

TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

BOARD_LDPI_RECOVERY := true
BOARD_HAS_JANKY_BACKBUFFER := true

TARGET_PREBUILT_KERNEL := device/htc/click/custom/kernel

# Wifi related defines
BOARD_WPA_SUPPLICANT_DRIVER := CUSTOM
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := libWifiApi
BOARD_WLAN_DEVICE           := wl1251
BOARD_WLAN_TI_STA_DK_ROOT   := system/wlan/ti/sta_dk_4_0_4_32
WIFI_DRIVER_MODULE_PATH     := "/system/lib/modules/wlan.ko"
WIFI_DRIVER_MODULE_ARG      := ""
WIFI_DRIVER_MODULE_NAME     := "wlan"
WIFI_FIRMWARE_LOADER        := "wlan_loader"

TARGET_PROVIDES_INIT_RC := false

BOARD_KERNEL_CMDLINE := no_console_suspend=1
BOARD_KERNEL_BASE := 0x02E00000
BOARD_KERNEL_PAGESIZE := 2048

BOARD_USES_GENERIC_AUDIO := false

BOARD_HAVE_BLUETOOTH := true

BOARD_AVOID_DRAW_TEXTURE_EXTENSION := true

BOARD_VENDOR_USE_AKMD := akm8973

BOARD_VENDOR_QCOM_AMSS_VERSION := 1355

# Change for Gallery 3D or not
BOARD_HAS_LIMITED_EGL := true

TARGET_HARDWARE_3D := false

# OpenGL drivers config file path
BOARD_EGL_CFG := device/htc/click/custom/egl.cfg

# No authoring clock for OpenCore
# BOARD_NO_PV_AUTHORING_CLOCK := true

BOARD_HAVE_FM_RADIO := true
BOARD_GLOBAL_CFLAGS += -DHAVE_FM_RADIO

BOARD_USES_QCOM_HARDWARE := true
BOARD_USES_QCOM_GPS := true
BOARD_USES_QCOM_LIBS := true
BOARD_USES_GPSSHIM := true
BOARD_GPS_LIBRARIES := libgps librpc

BOARD_USE_NEW_LIBRIL_HTC := true

TARGET_LIBAGL_USE_GRALLOC_COPYBITS := true

TARGET_ELECTRONBEAM_FRAMES := 10

# WITH_DEXPREOPT := true
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
# Changed for click
BOARD_BOOTIMAGE_PARTITION_SIZE := 0x00280000
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 0x00500000
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 0x09600000
BOARD_USERDATAIMAGE_PARTITION_SIZE := 0x0a520000
BOARD_FLASH_BLOCK_SIZE := 131072

# Add LUNFILE configuration to the system
# BOARD_UMS_LUNFILE := "/sys/devices/platform/usb_mass_storage/lun0/file"
