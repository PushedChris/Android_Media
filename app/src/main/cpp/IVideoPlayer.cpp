//
// Created by comochirs on 2/14/21.
//
#include "IVideoPlayer.h"

void IVideoPlayer::renderSurface(uint8_t *pixel) {

    ANativeWindow_acquire(mANativeWindow);

    if (0 != ANativeWindow_lock(mANativeWindow, &nwBuffer, NULL)) {
        LOGV("ANativeWindow_lock() error");
        return;
    }
    if (nwBuffer.width >= nwBuffer.stride) {
        memcpy(nwBuffer.bits, pixel, nwBuffer.width * nwBuffer.height * 2);
    } else {
        LOGV("ANativeWindow new buffer width is %d,height is %d ,stride is %d",
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
int32_t IVideoPlayer::setSize(int32_t width, int32_t height) {
    int32_t format = WINDOW_FORMAT_RGB_565;
    LOGV("ANativeWindow setSize width is %d,height is %d\n",
         width, height);
    if (NULL == mANativeWindow) {
        LOGV("mANativeWindow is NULL.");
        return -1;
    }

    return ANativeWindow_setBuffersGeometry(mANativeWindow, width, height,
                                            format);
}

void IVideoPlayer::setWindow(ANativeWindow *_win) {
    mIsReady = true;
    mANativeWindow = _win;
}

bool IVideoPlayer::isReady() {
    return mIsReady;
}

IVideoPlayer::IVideoPlayer() {
    mIsReady = false;
    mANativeWindow = nullptr;
}
