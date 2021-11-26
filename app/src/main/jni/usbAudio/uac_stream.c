//
// Created by Admin on 2021/11/11.
//
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "uac_stream.h"
#include "../utilbase.h"

#define LOGD(...) \
    __android_log_print(ANDROID_LOG_DEBUG, "audio_stream", __VA_ARGS__)

void _uac_stream_callback(struct libusb_transfer *transfer) {
    uac_stream_handler_t *uac_handler = transfer->user_data;
//    LOGD("Audio loop: uac_handler->running %d", uac_handler->running);
    switch (transfer->status) {
        case LIBUSB_TRANSFER_COMPLETED:
            if (transfer->num_iso_packets) {
                /* This is an isochronous mode transfer, so each packet has a payload transfer */
                _uac_process_payload_iso(transfer, uac_handler);
            }
            break;
        case LIBUSB_TRANSFER_NO_DEVICE:
            uac_handler->running = 0;
            LOGD("_uac_stream_callback NO_DEVICE:%d\n", uac_handler->running);
            // this needs for unexpected disconnect of cable otherwise hangup
            // pass through to following lines
        case LIBUSB_TRANSFER_CANCELLED:
        case LIBUSB_TRANSFER_ERROR:
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
        case LIBUSB_TRANSFER_STALL:
        case LIBUSB_TRANSFER_OVERFLOW:
            break;
    }
}

void _uac_process_payload_iso(struct libusb_transfer *transfer, uac_stream_handler_t *uac_handler) {
    unsigned int i;
    int len = 0;
    int maxLen = 0;

    // Get an env handle
    JNIEnv *env;
    void *void_env;
    struct audio_byte_object *object = uac_handler->audio_object;
    JavaVM *java_vm = object->vm;
    bool had_to_attach = false;
    jint status = (*java_vm)->GetEnv(java_vm, &void_env, JNI_VERSION_1_6);

    if (status == JNI_EDETACHED) {
        (*java_vm)->AttachCurrentThread(java_vm, &env, NULL);
        had_to_attach = true;
    } else {
        env = void_env;
    }
    // Create a jbyteArray.

    //杂音问题找到
    for (int j = 0; j < transfer->num_iso_packets; ++j) {
        struct libusb_iso_packet_descriptor *pack = &transfer->iso_packet_desc[j];
        maxLen += pack->actual_length;
//        LOGD("libusb_iso_packet_descriptor actual_length:%d length:%d",
//             pack->actual_length,
//             pack->length);
    }
    if (maxLen <= 0) {
        LOGD("Error :maxLen is %d ", maxLen);
        goto out;
    }
    jbyteArray audioByteArray = (*env)->NewByteArray(env, maxLen);


    for (i = 0; i < transfer->num_iso_packets; i++) {

        struct libusb_iso_packet_descriptor *pack = &transfer->iso_packet_desc[i];

        if (pack->status != LIBUSB_TRANSFER_COMPLETED) {
            LOGD("libusb_iso_packet_descriptor actual_length:%d length:%d",
                 pack->actual_length,
                 pack->length);
            LOGD("Error (status %d: %s) errno: %s:", pack->status,
                 libusb_error_name(pack->status), strerror(errno));
            /* This doesn't happen, so bail out if it does. */
            goto out;
        }
        const uint8_t *data = libusb_get_iso_packet_buffer_simple(transfer, i);
        (*env)->SetByteArrayRegion(env, audioByteArray, len, pack->actual_length, (jbyte *) data);
        len += pack->actual_length;
    }
    // Call write()
    (*env)->CallVoidMethod(env, object->audioObject,
                           object->pcmData, audioByteArray, maxLen);
    (*env)->DeleteLocalRef(env, audioByteArray);
    if ((*env)->ExceptionCheck(env)) {
        LOGD("Exception while trying to pass sound data to java");
        return;
    }
//    num_bytes += len;
//    num_xfer++;

    out:
    if (had_to_attach) {
        (*java_vm)->DetachCurrentThread(java_vm);
    }

    if (uac_handler->running) {
        if (libusb_submit_transfer(transfer) < 0) {
            LOGD("error re-submitting URB\n");
            exit(1);
        }
    }
}