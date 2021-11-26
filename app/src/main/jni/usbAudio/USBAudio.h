//
// Created by Admin on 2021/11/10.
//

#ifndef UVCCAMERA_USBAUDIO_H
#define UVCCAMERA_USBAUDIO_H

#include <jni.h>
#include <pthread.h>
#include "../libusb/libusb/libusb.h"

extern "C" {
#include "uac_stream.h"
}


#define DEFAULT_SIMPLING_RATE 48000
#define NUM_TRANSFERS 10
#define NUM_PACKETS 10
#define PACKET_SIZE 192

static unsigned long num_bytes = 0, num_xfer = 0;
static struct timeval tv_start;

/** UAC request code (A.8) */
enum uac_req_code {
    UAC_RC_UNDEFINED = 0x00,
    UAC_SET_CUR = 0x01,
    UAC_GET_CUR = 0x81
};

class USBAudio {
private:
    const struct libusb_interface *i_face;
    const struct libusb_interface_descriptor *if_desc;
    struct libusb_config_descriptor *uac_config;
    struct libusb_context *uac_ctx;
    struct libusb_device_handle *uac_devh;

    struct audio_streaming_descriptor *audio_stream_desc;

    struct libusb_device *uac_dev;
    char *mUsbFs;
    int sample_rate;
    int mMaxPacketSize = PACKET_SIZE;
    int _controlInterface = 0;
    int _speakerInterface = 3;
    int _alternateSetting = 1;
    uint8_t _speakerEndpoint = 1;
    uac_stream_handler_t *uac_handler;


    int interface_claim_if(libusb_device_handle *devh);

    int scan_audio_interface(libusb_device *usbDev);

    int operate_interface(libusb_device_handle *devh, int interface_number);

    int fill_iso_transfer();

    int set_sample_rate_v1(int rate);

    void set_audio_stream_desc(const libusb_interface_descriptor *desc);

public:
    USBAudio();

    void closeAudio();

    int initAudio(int vid, int pid, int busnum, int devaddr, int fd, const char *usbfs);

    void setCallback(JavaVM *vm, JNIEnv *env, jobject callback_obj);

    int startCapture();

    int stopCapture();

    int getSampleRate();

    int getChannelCount();

    int getBitResolution();

    bool isRunning();

};

#endif //UVCCAMERA_USBAUDIO_H