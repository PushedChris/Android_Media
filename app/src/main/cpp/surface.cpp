//
// Created by comochirs on 2/14/21.
//

#include "com_comochris_mediaplayer.h"
#include "player.h"

// for native window JNI
#include <android/native_window_jni.h>
#include <android/native_window.h>

static ANativeWindow* mANativeWindow;
static ANativeWindow_Buffer nwBuffer;

static jclass globalVideoSurfaceClass = NULL;
static jobject globalVideoSurfaceObject = NULL;

void renderSurface(uint8_t *pixel) {

    if (global_context.pause) {
        return;
    }

    ANativeWindow_acquire(mANativeWindow);

    if (0 != ANativeWindow_lock(mANativeWindow, &nwBuffer, NULL)) {
        LOGV("ANativeWindow_lock() error");
        return;
    }
    if (nwBuffer.width >= nwBuffer.stride) {
        memcpy(nwBuffer.bits, pixel, nwBuffer.width * nwBuffer.height * 2);
    } else {
        LOGV("new buffer width is %d,height is %d ,stride is %d",
             nwBuffer.width, nwBuffer.height, nwBuffer.stride);
        int i;
        for (i = 0; i < nwBuffer.height; ++i) {
            memcpy((void*) ((int *) nwBuffer.bits + nwBuffer.stride * i * 2),
                   (void*) ((int *) pixel + nwBuffer.width * i * 2),
                   nwBuffer.width * 2);
        }
    }

    if (0 != ANativeWindow_unlockAndPost(mANativeWindow)) {
        LOGV("ANativeWindow_unlockAndPost error");
        return;
    }

    ANativeWindow_release(mANativeWindow);
}

// format not used now.
int32_t setBuffersGeometry(int32_t width, int32_t height) {
    int32_t format = WINDOW_FORMAT_RGB_565;

    if (NULL == mANativeWindow) {
        LOGV("mANativeWindow is NULL.");
        return -1;
    }

    return ANativeWindow_setBuffersGeometry(mANativeWindow, width, height,
                                            format);
}

JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_setSurface(JNIEnv *env, jobject thiz, jobject view) {
    pthread_t thread_1;

    //LOGV("fun env is %p", env);

    jclass localVideoSurfaceClass = env->FindClass(
            "com/comochris/mediaplayer/FFMediaPlayer");
    if (NULL == localVideoSurfaceClass) {
        LOGV("FindClass VideoSurface failure.");
        return -1;
    }

    globalVideoSurfaceClass = (jclass) env->NewGlobalRef(
            localVideoSurfaceClass);
    if (NULL == globalVideoSurfaceClass) {
        LOGV("localVideoSurfaceClass to globalVideoSurfaceClass failure.");
    }

    globalVideoSurfaceObject = (jclass) env->NewGlobalRef(thiz);
    if (NULL == globalVideoSurfaceObject) {
        LOGV("obj to globalVideoSurfaceObject failure.");
    }

    if (NULL == view) {
        LOGV("surface is null, destroy?");
        mANativeWindow = NULL;
        return 0;
    }

    // obtain a native window from a Java surface
    mANativeWindow = ANativeWindow_fromSurface(env, view);
    LOGV("mANativeWindow ok");

    pthread_create(&thread_1, NULL, open_media, NULL);

    return 0;
}

JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_nativePausePlayer(JNIEnv *env, jobject thiz) {
    global_context.pause = 1;
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_nativeResumePlayer(JNIEnv *env, jobject thiz) {
    global_context.pause = 0;
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_nativeStopPlayer(JNIEnv *env, jobject thiz) {
    global_context.quit = 1;
    return 0;
}