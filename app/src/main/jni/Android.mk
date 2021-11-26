#include $(call all-subdir-makefiles)
PROJ_PATH	:= $(call my-dir)
include $(CLEAR_VARS)
include $(PROJ_PATH)/libusb/android/jni/Android.mk
include $(PROJ_PATH)/usbAudio/Android.mk

#ROOT_PATH := $(call my-dir)
#LOCAL_PATH := $(call my-dir)
#include $(CLEAR_VARS)
#LOCAL_MODULE	:= usbaudio
#LOCAL_SHARED_LIBRARIES += usb100
#LOCAL_LDLIBS := -llog
#LOCAL_SRC_FILES := usbaudio_dump.c
#
#include $(BUILD_SHARED_LIBRARY)
#include $(LOCAL_PATH)/libusb/android/jni/Android.mk

# include $(CLEAR_VARS)
# LOCAL_MODULE := libusb-1.0
# LOCAL_SRC_FILES := libusb-1.0/lib/armeabi-v7a/libusb-1.0.so
# LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libusb-1.0/include/libusb-1.0
# LOCAL_EXPORT_LDLIBS := -llog
# include $(PREBUILT_SHARED_LIBRARY)
#
#
# include $(CLEAR_VARS)
# LOCAL_MODULE	:= usbaudio
# LOCAL_SHARED_LIBRARIES := libusb-1.0
# LOCAL_LDLIBS := -llog
# #LOCAL_CFLAGS :=
# LOCAL_SRC_FILES := usbaudio_dump.c
# include $(BUILD_SHARED_LIBRARY)

# include $(CLEAR_VARS)
# LOCAL_MODULE := libusb
# LOCAL_SRC_FILES := libusb/android/libs/$(TARGET_ARCH_ABI)/libusb100.so
# LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libusb/libusb
# LOCAL_EXPORT_LDLIBS := -llog
# include $(PREBUILT_SHARED_LIBRARY)
#
#
# include $(CLEAR_VARS)
# LOCAL_MODULE	:= usbaudio
# LOCAL_SHARED_LIBRARIES := libusb
# LOCAL_LDLIBS := -llog
# #LOCAL_CFLAGS :=
# LOCAL_SRC_FILES := usbaudio_dump.c
# include $(BUILD_SHARED_LIBRARY)