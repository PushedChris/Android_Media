//
// Created by comochirs on 2/14/21.
//
#ifndef MEDIAPLAYER_COM_COMOCHRIS_MEDIAPLAYER_CPP
#define MEDIAPLAYER_COM_COMOCHRIS_MEDIAPLAYER_CPP
#include <jni.h>

#include "MediaPlayerController.h"

#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/log.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
//#define TESTFILEPATH "/storage/emulated/0/DCIM/Camera/abc.mp4"

ANativeWindow *window = NULL;
MediaPlayerController *controller = NULL;

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_initPlayer(JNIEnv *env, jobject thiz){
    LOGV("initPlayer");
    controller = new MediaPlayerController();
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_destoryPlayer(JNIEnv *env, jobject thiz){
    LOGV("destoryPlayer");
    if(NULL != controller){
        delete(controller);
    }
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_startPlayer(JNIEnv *env, jobject thiz){
    if(NULL == controller){
        LOGV("controller is NULL");
        return -1;
    }
    controller->start();
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_stopPlayer(JNIEnv *env, jobject thiz){
    if(NULL == controller){
        LOGV("controller is NULL");
        return -1;
    }
    controller->stop();
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_openMedia(JNIEnv *env, jobject thiz,jstring filePath){
    if(NULL == controller){
        LOGV("controller is NULL");
        return -1;
    }
    const char *path = env->GetStringUTFChars(filePath,NULL);
    bool result = controller->openFile(path);
    //bool result = controller->openFile(TESTFILEPATH);
    env->ReleaseStringUTFChars(filePath,path);
    return result;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_closeMedia(JNIEnv *env, jobject thiz){
    if(NULL == controller){
        LOGV("controller is NULL");
        return -1;
    }
    controller->closeFile();
    return  0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_setSurface(JNIEnv *env, jobject thiz,jobject view){
    if(NULL == controller){
        LOGV("controller is NULL");
        return -1;
    }
    LOGV("FFMediaPlayer_setSurface");
    window = ANativeWindow_fromSurface(env,view);
    controller->setWindow(window);
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_setSize(JNIEnv *env, jobject thiz,jint width,jint height){
    if(NULL == controller){
        LOGV("controller is NULL");
        return -1;
    }
    LOGV("controller set width: %d,height : %d",width,height);
    controller->setSize(width,height);
    return 0;
}
#endif //MEDIAPLAYER_COM_COMOCHRIS_MEDIAPLAYER_CPP
