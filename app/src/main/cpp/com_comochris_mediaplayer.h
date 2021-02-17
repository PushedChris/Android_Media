//
// Created by comochirs on 2/14/21.
//
#include <jni.h>
#ifndef MEDIAPLAYER_COM_COMOCHRIS_MEDIAPLAYER_H
#define MEDIAPLAYER_COM_COMOCHRIS_MEDIAPLAYER_H
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jint
JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_setSurface(JNIEnv *env, jobject thiz, jobject view);


JNIEXPORT jint
JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_nativePausePlayer(JNIEnv *env, jobject thiz);


JNIEXPORT jint
JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_nativeResumePlayer(JNIEnv *env, jobject thiz);


JNIEXPORT jint
JNICALL
Java_com_comochris_mediaplayer_FFMediaPlayer_nativeStopPlayer(JNIEnv *env, jobject thiz);

#ifdef __cplusplus
};
#endif
#endif //MEDIAPLAYER_COM_COMOCHRIS_MEDIAPLAYER_H
