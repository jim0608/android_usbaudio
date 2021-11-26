LOCAL_PATH := $(call my-dir)



include $(CLEAR_VARS)

LOCAL_MODULE	:= USBAudio
LOCAL_SHARED_LIBRARIES += libusb100
LOCAL_LDLIBS := -llog

LOCAL_SRC_FILES := \
	uac_stream.c\
	USBAudio.cpp\
	native_usb_audio.cpp

#LOCAL_SRC_FILES := usbaudio_dump.c

include $(BUILD_SHARED_LIBRARY)

