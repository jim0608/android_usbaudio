//
// Created by Admin on 2021/11/11.
//

#include <jni.h>
#include <stdint.h>
#include <pthread.h>
#include "../libusb/libusb/libusb.h"

#ifndef UACCAMERA_UAC_STREAM_H
#define UACCAMERA_UAC_STREAM_H


struct audio_byte_object {
    JavaVM *vm;
    JNIEnv *env;
    jclass audio_class;
    jobject audioObject;
    jmethodID pcmData;
};

struct uac_stream_handler {
    int running;
    struct audio_byte_object *audio_object;
};

//audio_streaming_descriptor
struct audio_streaming_descriptor {
    //该结构体的字节度长，
    uint8_t bLength;
    //描述符类型CS_INTERFACE，值为0x24
    uint8_t bDescriptorType;
    //描述符子类型 FORMAT_TYPE
    uint8_t bDescriptorSubtype;
    //音频数据格式，这里为FORMAT_TYPE_I
    uint8_t bFormatType;
    //音频数据的通道数
    uint8_t bNrChannels;
    //每通道数据的字节数，可以1，2，3，4
    uint8_t bSubframeSize;
    //bSubframeSize中的有效位数。
    uint8_t bBitResolution;
    //在当前interface下有多少种采样率
    uint8_t bSamFreqType;
};




typedef struct uac_stream_handler uac_stream_handler_t;

int fill_iso_transfer(int _speakerEndpoint);

void _uac_stream_callback(struct libusb_transfer *transfer);

void _uac_process_payload_iso(struct libusb_transfer *transfer, uac_stream_handler_t *uac_handler);


#endif //UACCAMERA_UAC_STREAM_H
