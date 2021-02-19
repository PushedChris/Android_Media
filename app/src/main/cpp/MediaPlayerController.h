//
// Created by comochirs on 2/18/21.
//

#ifndef MEDIAPLAYER_MEDIAPLAYERCONTROLLER_H
#define MEDIAPLAYER_MEDIAPLAYERCONTROLLER_H


#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <thread>

#include "IAVBlockQueue.h"
#include "IAVDecoder.h"
#include "OpenSLESPlayer.h"
#include "IVideoPlayer.h"
//#include "OpenSLESPlayer.h"
#include "AVQueueInterface.h"
using namespace std;

class MediaPlayerController : public AVQueueInterface,public IAudioFrameProvider, public IVideoFrameProvider{
public:
    MediaPlayerController();
    ~MediaPlayerController();

    bool openFile(const char *path);

    void closeFile();

    void start();

    void stop();

    void setWindow(ANativeWindow *window);

    void setSize(int width, int height);

    int64_t getDuration();

    void receiveAudioFrame(AudioFrame *audioData) override;

    void receiveVideoFrame(VideoFrame *videoData) override;

    AudioFrame *getUsedAudioFrame() override;

    VideoFrame *getUsedVideoFrame() override;

    void putUsedAudioFrame(AudioFrame *audioData) override;

    void putUsedVideoFrame(VideoFrame *videoData) override;

    VideoFrame *getVideoFrame() override ;

    void putBackUsed(VideoFrame* data) override ;

    AudioFrame *getAudioFrame() override;

    void putBackUsed(AudioFrame* data) override ;

private:

    void discardAllFrame();

    IAVDecoder *decoder = NULL;

    OpenSLESPlayer *audioPlayer = NULL;
    IVideoPlayer *videoPlayer = NULL;

    IAVBlockQueue<AudioFrame *> *audioQueue = NULL;
    IAVBlockQueue<VideoFrame *> *videoQueue = NULL;

    VideoFrame *nextVFrame = NULL;
    VideoFrame *waitVFrame = NULL;
    mutex videoMu;

    int64_t currentPositionMS = 0;
    mutex currentPosMu;
    condition_variable updateCurrentPosSignal;

    thread *imageRefreshThread = NULL;

    bool exitFlag = false;
};


#endif //MEDIAPLAYER_MEDIAPLAYERCONTROLLER_H
