//
// Created by Admin on 2021/11/10.
//

#include <jni.h>
#include <android/log.h>
#include "../utilbase.h"

#include "USBAudio.h"

#define UNUSED __attribute__((unused))

static JavaVM *java_vm = NULL;

jlong setField_long(JNIEnv *pEnv, jobject pJobject, const char *field_name, jlong val);


extern "C" JNIEXPORT jint

JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved UNUSED) {
    LOGD("libusbaudio: loaded");
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    java_vm = vm;

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void
JNI_OnUnload(JavaVM *vm, void *reserved UNUSED) {

}

extern "C" JNIEXPORT ID_TYPE
JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeCreate(JNIEnv *env, jobject thiz) {
    ENTER();
    auto *usbAudio = new USBAudio();
    jobject frame_callback_obj = env->NewGlobalRef(thiz);
    usbAudio->setCallback(java_vm, env, frame_callback_obj);
    setField_long(env, thiz, "mNativePtr", reinterpret_cast<ID_TYPE>(usbAudio));
    RETURN(reinterpret_cast<ID_TYPE>(usbAudio), ID_TYPE);
}


extern "C" JNIEXPORT void JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeClose(JNIEnv *env,
                                                    jobject thiz,
                                                    jlong id_camera) {
    ENTER();
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    if (LIKELY(audio)) {
        audio->closeAudio();
        SAFE_DELETE(audio);
    }
    EXIT()
}


extern "C" JNIEXPORT jint JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeInit(JNIEnv *env, jobject thiz,
                                                   jlong id_camera,
                                                   jint vid,
                                                   jint pid,
                                                   jint busnum,
                                                   jint devaddr,
                                                   jint fd,
                                                   jstring usbfs) {
    ENTER();
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    const char *c_usbfs = env->GetStringUTFChars(usbfs, JNI_FALSE);

    int r = 0;
    if (LIKELY(audio)) {
        r = audio->initAudio(vid, pid, busnum, devaddr, fd, c_usbfs);
    }
    env->ReleaseStringUTFChars(usbfs, c_usbfs);
    RETURN(r, jint)
}


extern "C" JNIEXPORT jint JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeGetSampleRate(JNIEnv *env,
                                                            jobject thiz,
                                                            jlong id_camera) {
    int r = 16000;
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    if (LIKELY(audio)) {
        r = audio->getSampleRate();
    }
    RETURN(r, jint)
}

extern "C" JNIEXPORT jint JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeGetChannelCount(JNIEnv *env,
                                                              jobject thiz,
                                                              jlong id_camera
) {
    int r = 1;
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    if (LIKELY(audio)) {
        r = audio->getChannelCount();
    }
    RETURN(r, jint)
}

extern "C" JNIEXPORT jint JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeGetBitResolution(JNIEnv *env,
                                                               jobject thiz,
                                                               jlong id_camera
) {
    int r = 16000;
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    if (LIKELY(audio)) {
        r = audio->getBitResolution();
    }
    RETURN(r, jint)
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeIsRunning(JNIEnv *env,
                                                        jobject thiz,
                                                        jlong id_camera) {
    bool isRunning= false;
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    if (LIKELY(audio)) {
        isRunning = audio->isRunning();
    }
    return isRunning;
}

extern "C" JNIEXPORT jint
JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeStartCapture(JNIEnv *env,
                                                           jobject thiz,
                                                           jlong id_camera
) {
    int r = -1;
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    if (LIKELY(audio)) {
        r = audio->startCapture();
    }

    RETURN(r, jint);
}
extern "C" JNIEXPORT jint
JNICALL
Java_au_id_jms_usbaudio_USBAudio_nativeStopCapture(JNIEnv *env,
                                                          jobject thiz,
                                                          jlong id_camera
) {
    int r = 0;
    auto *audio = reinterpret_cast<USBAudio *>(id_camera);
    if (LIKELY(audio)) {
        r = audio->stopCapture();
    }
    RETURN(r, jint);
}


/**
 * set the value into the long field
 * @param env: this param should not be null
 * @param bullet_obj: this param should not be null
 * @param field_name
 * @params val
 */

jlong setField_long(JNIEnv *env,
                    jobject java_obj,
                    const char *field_name,
                    jlong val) {
#if LOCAL_DEBUG
    LOGV("setField_long:");
#endif

    jclass clazz = env->GetObjectClass(java_obj);
    jfieldID field = env->GetFieldID(clazz, field_name, "J");
    if (LIKELY(field))
        env->SetLongField(java_obj, field, val);
    else {
        LOGE("__setField_long:field '%s' not found", field_name);
    }
#ifdef ANDROID_NDK
    env->DeleteLocalRef(clazz);
#endif
    return val;
}

