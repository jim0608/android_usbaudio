//
// Created by Admin on 2021/11/2.
//
#include <jni.h>

#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <stdbool.h>
#include <android/log.h>
#include <cstring>
#include "../libusb/libusb/libusb.h"
#include "USBAudio.h"




#define LOGD(...) \
    __android_log_print(ANDROID_LOG_DEBUG, "USBAudio", __VA_ARGS__)

#define UNUSED __attribute__((unused))


int USB_REQ_CS_ENDPOINT_SET = // 0x22
        LIBUSB_ENDPOINT_OUT |
        LIBUSB_REQUEST_TYPE_CLASS |
        LIBUSB_RECIPIENT_ENDPOINT;
int USB_REQ_CS_ENDPOINT_GET = // 0xa2
        LIBUSB_ENDPOINT_IN |
        LIBUSB_REQUEST_TYPE_CLASS |
        LIBUSB_RECIPIENT_ENDPOINT;

//private
int USBAudio::interface_claim_if(libusb_device_handle *devh) {
    int r = 0;
//    operate_interface(devh, _controlInterface);
    r = operate_interface(devh, _speakerInterface);
    if (r < 0) {
        return r;
    }
    LOGD("Select the  bEndpointAddress:%d\n", if_desc->endpoint->bEndpointAddress);
    LOGD("Select the altsetting:%d, _speakerInterface:%d\n", _alternateSetting, _speakerInterface);
    r = libusb_set_interface_alt_setting(devh, _speakerInterface, _alternateSetting);
    if (r != 0) {
        return -1;
    }

    return 0;
}

int USBAudio::scan_audio_interface(libusb_device *usbDev) {
    int r = 0;
    r = libusb_get_config_descriptor(usbDev, 0, &uac_config);
    LOGD("scan_audio_interface");
    for (int interface_idx = 0; interface_idx < uac_config->bNumInterfaces; interface_idx++) {
        i_face = &uac_config->interface[interface_idx];
        if (i_face->altsetting->bInterfaceClass != LIBUSB_CLASS_AUDIO/*1*/) {// Audio, Control
            continue;
        }
        LOGD("scan_audio_interface :%d", i_face->num_altsetting);

        for (int i = 0; i < i_face->num_altsetting; ++i) {
            if_desc = &i_face->altsetting[i];

            switch (if_desc->bInterfaceSubClass) {
                case 1:
                    _controlInterface = if_desc->bInterfaceNumber;
                    break;
                case 2:
                    if (if_desc->bNumEndpoints) {

                        auto endpoint = if_desc->endpoint;
                        if ((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) ==
                            LIBUSB_ENDPOINT_IN) {
                            LOGD("set _speakerInterface ");
                            //获取 FORMAT_TYPE_I 相关数据
                            set_audio_stream_desc(if_desc);
                            //赋值
                            _speakerInterface = if_desc->bInterfaceNumber;
                            _alternateSetting = if_desc->bAlternateSetting;
                            _speakerEndpoint = endpoint->bEndpointAddress;
                            mMaxPacketSize = endpoint->wMaxPacketSize;
                            LOGD(" _speakerInterface %d _controlInterface %d mMaxPacketSize %d _alternateSetting %d\n",
                                 _speakerInterface,
                                 _controlInterface,
                                 mMaxPacketSize,
                                 _alternateSetting);
                        }
                    }
                    break;
            }
        }
    }
    libusb_free_config_descriptor(uac_config);
    return r;
}

void USBAudio::set_audio_stream_desc(const libusb_interface_descriptor *desc) {
    const unsigned char *if_audio_stream_desc = desc->extra;
    auto if_audio_stream_desc_size = desc->extra_length;

    //Remove AudioStreaming Interface Descriptor AS_GENERAL
    int extra_offset = if_audio_stream_desc[0];
    char *buffer = (char *) (if_audio_stream_desc + extra_offset);

    //AudioStreaming Interface Descriptor FORMAT_TYPE
    //Get AudioStreaming FORMAT_TYPE/FORMAT_TYPE_I About Audio Information
    char *desc_buffer = (char *) malloc(8);
    memcpy(desc_buffer, buffer, 8);

    //Force desc buffer into audio streaming descriptor
    audio_stream_desc = reinterpret_cast<audio_streaming_descriptor *>(desc_buffer);

    //Get Audio Sample Rates
    char *sam_freq_buffer = (char *) (buffer + 8);

    for (int j = 0; j < audio_stream_desc->bSamFreqType; ++j) {
        char data[3];
        int index = j * 3;
        data[0] = sam_freq_buffer[index];
        data[1] = sam_freq_buffer[index + 1];
        data[2] = sam_freq_buffer[index + 2];
        int rate = data[0] | (data[1] << 8) | (data[2] << 16);
        LOGD("Get desc rate %d\n", rate);
        if (rate <= 48000) {
            sample_rate = rate;
        }
    }
}

int USBAudio::operate_interface(libusb_device_handle *devh, int interface_number) {
    int r = 0;

    //detach_kernel_driver
    r = libusb_kernel_driver_active(devh, interface_number);
    if (r == 1) { //find out if kernel driver is attached
        LOGD("Kernel Driver Active\n");
        if (libusb_detach_kernel_driver(devh, interface_number) == 0) //detach it
            LOGD("Kernel Driver Detached!\n");
    }
    LOGD("kernel detach interface_number:%d\n", interface_number);

    //claim_interface
    r = libusb_claim_interface(devh, interface_number);
    LOGD("claim_interface r:%s\n", libusb_error_name(r));
    if (r != 0) {
        LOGD("Error claiming interface: %s\n", libusb_error_name(r));
        return r;
    }
    LOGD("claim_interface r:%d\n", r);
    return r;
}

int USBAudio::fill_iso_transfer() {
    int r = 0;
//    mMaxPacketSize = 32;
    int endpoint_bytes_per_packet = mMaxPacketSize;
    int packets_per_transfer = NUM_PACKETS;
    int total_transfer_size = mMaxPacketSize * NUM_PACKETS;
    int transfer_id = 0;
    unsigned char *transfer_bufs[NUM_TRANSFERS];
    struct libusb_transfer *transfer;
    struct libusb_transfer *transfers[NUM_TRANSFERS];


    LOGD("Set up the transfers\n");

    LOGD("before fill EndpointAddress:%d, per_packet:%d, packets:%d, total_transfer_size:%d\n",
         _speakerEndpoint, endpoint_bytes_per_packet, packets_per_transfer, total_transfer_size);
    for (transfer_id = 0; transfer_id < NUM_TRANSFERS; ++transfer_id) {
        transfer = (struct libusb_transfer *) libusb_alloc_transfer(packets_per_transfer);
        transfers[transfer_id] = transfer;
        transfer_bufs[transfer_id] = (unsigned char *) malloc(total_transfer_size);
        memset(transfer_bufs[transfer_id], 0, total_transfer_size);
        libusb_fill_iso_transfer(transfer, uac_devh,
                                 _speakerEndpoint,
                                 transfer_bufs[transfer_id], total_transfer_size,
                                 packets_per_transfer, _uac_stream_callback,
                                 (void *) uac_handler, 0);

        libusb_set_iso_packet_lengths(transfer, endpoint_bytes_per_packet);

    }
    for (transfer_id = 0; transfer_id < NUM_TRANSFERS; transfer_id++) {
        r = libusb_submit_transfer(transfers[transfer_id]);
        if (r != 0) {
            LOGD("libusb_submit_transfer failed: %s, errno:%s\n",
                 libusb_error_name(r),
                 strerror(errno));
            break;
        }
    }

    if (r != 0) {
        return r;
    }

    uac_handler->running = 0;

    return r;
}

int USBAudio::set_sample_rate_v1(int rate) {
    unsigned char data[3];
    int ret, crate;

    data[0] = (rate & 0xff);
    data[1] = (rate >> 8);
    data[2] = (rate >> 16);


    ret = libusb_control_transfer(uac_devh,
                                  USB_REQ_CS_ENDPOINT_SET,
                                  UAC_SET_CUR,
                                  0x0100,
                                  _speakerEndpoint,
                                  data, sizeof(data), 500);
    if (ret < 0) {
        LOGD("%d:%d: cannot set freq %d to ep %#x\n",
             _speakerInterface, _alternateSetting, rate, _speakerEndpoint);
        return ret;
    }

    ret = libusb_control_transfer(uac_devh,
                                  USB_REQ_CS_ENDPOINT_GET,
                                  UAC_GET_CUR,
                                  0x0100,
                                  _speakerEndpoint,
                                  data, sizeof(data), 500);

    if (ret < 0) {
        LOGD("%d:%d: cannot get freq at ep %#x\n",
             _speakerInterface, _alternateSetting, _speakerEndpoint);
        /* some devices don't support reading */
    }


    crate = data[0] | (data[1] << 8) | (data[2] << 16);
    LOGD("host rate is %d ,device rate is %d\n", rate, crate);
    if (!crate) {
        LOGD("failed to read current rate; disabling the check\n");
        return 0;
    }

    if (crate != rate) {
        LOGD("current rate %d is different from the runtime rate %d\n", crate, rate);
        // runtime->rate = crate;
    }

    return 0;
}


//public
USBAudio::USBAudio() :
        sample_rate(DEFAULT_SIMPLING_RATE) {}

int USBAudio::initAudio(int vid, int pid, int busnum, int devaddr,
                        int fd, const char *usbfs) {
    int ret = 0;
    //for return values
    LOGD("before 11111 vid:%d pid:%d\n", vid, pid);
    strdup(usbfs);
    fd = dup(fd);
    LOGD("before 11111 ret:%s\n", libusb_error_name(ret));
    ret = libusb_init2(&uac_ctx, usbfs);         //initialize a library session
    LOGD("before 11111 ret:%s\n", libusb_error_name(ret));
    if (ret < 0) {
        LOGD("Init Error \n"); //there was an error
        return ret;
    }
    LOGD("before 11111 vid:%d pid:%d\n", vid, pid);
    uac_dev = libusb_get_device_with_fd(uac_ctx, vid, pid, NULL, fd, busnum, devaddr);
    if (uac_dev) {
        ret = libusb_set_device_fd(uac_dev,
                                   fd);  // assign fd to libusb_device for non-rooted Android devices
        libusb_ref_device(uac_dev);
    } else {
        return -1;
    }

    LOGD("open device err %s\n", libusb_error_name(ret));
    ret = libusb_open(uac_dev, &uac_devh);

    if (ret != LIBUSB_SUCCESS) {
        LOGD("open device err %d\n", ret);
        return ret;
    }

    //libusb_reset_device(dev_handle);
    LOGD("before scan_audio_interface ret:%s\n", libusb_error_name(ret));

    //scan interface
    ret = scan_audio_interface(uac_dev);
    if (ret < 0) {
        LOGD("scan_audio_interface err: ret:%s\n", libusb_error_name(ret));
        return ret;
    }

    //claim_interface and set_interface_alt_setting
    ret = interface_claim_if(uac_devh);
    if (ret < 0) {
        return ret;
    }
    LOGD("set mic config fail %d libusb:%s,errno:%s\n", ret, libusb_error_name(ret),
         strerror(errno));

    //Set sample rate
    LOGD("before sample_rate:%d\n", sample_rate);
    ret = set_sample_rate_v1(sample_rate);
    if (ret < 0) {
        LOGD("%d:%d: cannot get freq at ep %#x\n",
             _speakerInterface, _alternateSetting, _speakerEndpoint);
        /* some devices don't support reading */
        return ret;
    }

    LOGD("before interface_claim_if ret:%s\n", libusb_error_name(ret));
    ret = fill_iso_transfer();
    return ret;
}


void USBAudio::setCallback(JavaVM *vm, JNIEnv *env, jobject callback_obj) {
    uac_handler = new uac_stream_handler_t;
    uac_handler->audio_object = new audio_byte_object;
    if (vm) {
        uac_handler->audio_object->vm = vm;
    }
    if (env) {
        uac_handler->audio_object->env = env;
    }
//    if (uac_handler->audio_object->audioObject) {
//        env->DeleteGlobalRef(uac_handler->audio_object->audioObject);
//    }

    uac_handler->audio_object->audioObject = callback_obj;
    if (callback_obj) {
        // get method IDs of Java object for callback
        uac_handler->audio_object->audio_class = env->GetObjectClass(callback_obj);

        if (uac_handler->audio_object->audio_class) {
            uac_handler->audio_object->pcmData = env->GetMethodID(uac_handler->audio_object->audio_class, "pcmData",
                                                      "([B)V");
        } else {
            LOGD("failed to get object class");
        }

        env->ExceptionClear();
        if (!uac_handler->audio_object->pcmData) {
            LOGD("Can't find IFrameCallback#onFrame");
            env->DeleteGlobalRef(callback_obj);
            uac_handler->audio_object->audioObject = callback_obj = NULL;
        }
    }
}

int USBAudio::getSampleRate() {
    return sample_rate;
}

int USBAudio::getChannelCount() {
    return audio_stream_desc->bNrChannels;
}

int USBAudio::getBitResolution() {
    return audio_stream_desc->bBitResolution;
}

bool USBAudio::isRunning() {
    LOGD("Audio loop: isRunning %d", uac_handler->running);
    return uac_handler->running == 1;
}

//Java 调用
int USBAudio::startCapture() {
    int r = 0;
    LOGD("Audio loop: startCapture %d", uac_handler->running);
    if (uac_handler->audio_object->audioObject) {
        uac_handler->running = 1;
    }
    LOGD("Audio loop: audioObject %p", uac_handler->audio_object->audioObject);
    while (uac_handler->running) {
        int rc = libusb_handle_events(uac_ctx);
        if (rc != LIBUSB_SUCCESS) {
            LOGD("Audio loop: %d", rc);
            break;
        }
    }
    return r;
}


int USBAudio::stopCapture() {
    uac_handler->running = 0;
    LOGD("Audio loop: destroyAudio stopCapture%d", uac_handler->running);
    return 0;
}

void USBAudio::closeAudio() {
    LOGD("Audio loop: destroyAudio %d", uac_handler->running);
    stopCapture();
    if (uac_handler->audio_object->audioObject) {
        uac_handler->audio_object->env->DeleteGlobalRef(uac_handler->audio_object->audioObject);
    }
    libusb_release_interface(uac_devh, _speakerInterface);
    if (uac_devh)
        libusb_close(uac_devh);
    libusb_exit(uac_ctx);
}




