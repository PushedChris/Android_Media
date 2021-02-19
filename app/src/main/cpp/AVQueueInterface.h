//
// Created by comochirs on 2/18/21.
//

#ifndef MEDIAPLAYER_AVQUEUEINTERFACE_H
#define MEDIAPLAYER_AVQUEUEINTERFACE_H

#include "IAudioFrameProvider.h"
#include "IVideoFrameProvider.h"
extern "C"{
#include "libavutil/frame.h"
};

class AVQueueInterface {
public:
    virtual  void receiveAudioFrame(AudioFrame *audioData) = 0;
    virtual  void receiveVideoFrame(VideoFrame *videoData) = 0;

    virtual  AudioFrame *getUsedAudioFrame() = 0;
    virtual  VideoFrame *getUsedVideoFrame() = 0;

    virtual  void putUsedAudioFrame(AudioFrame *audioData) = 0;
    virtual  void putUsedVideoFrame(VideoFrame *videoData) = 0;
};


#endif //MEDIAPLAYER_AVQUEUEINTERFACE_H
