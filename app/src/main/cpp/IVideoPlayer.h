//
// Created by comochirs on 2/18/21.
//

#ifndef MEDIAPLAYER_IVIDEOPLAYER_H
#define MEDIAPLAYER_IVIDEOPLAYER_H

#include <memory>

// for native window JNI
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/log.h>

class IVideoPlayer{
public:
    IVideoPlayer();
    void renderSurface(uint8_t *pixel);
    int32_t setSize(int32_t width, int32_t height);
    void setWindow(ANativeWindow *_win);
    bool isReady();
private:
    ANativeWindow_Buffer nwBuffer;
    ANativeWindow *mANativeWindow;
    bool mIsReady;
};

#define TAG "FFmpeg"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#endif //MEDIAPLAYER_IVIDEOPLAYER_H
