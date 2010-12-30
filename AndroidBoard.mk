LOCAL_PATH := $(call my-dir)

file := $(TARGET_ROOT_OUT)/init.bahamas.rc
ALL_PREBUILT += $(file)
$(file) : $(LOCAL_PATH)/init.bahamas.rc | $(ACP)
	$(transform-prebuilt-to-target)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := bahamas-keypad.kcm
LOCAL_MODULE_TAGS := optional
include $(BUILD_KEY_CHAR_MAP)

# This will install the file in /system/etc
#

include $(CLEAR_VARS)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE := vold.fstab
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

-include device/htc/tattoo/AndroidBoardVendor.mk
