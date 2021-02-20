//
// Created by comochirs on 2/18/21.
//

#include "MediaPlayerController.h"
//#include "OpenSLESPlayer.h"
//#include "OpenGLESPlayer.h"
#include <android/log.h>

#define MODULE_NAME  "MediaPlayerController"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)



MediaPlayerController::MediaPlayerController() {
    LOGD("MediaPlayerController Constructor");
    decoder = new IAVDecoder();

    decoder->setDataReceiver(this);
    audioQueue = new IAVBlockQueue<AudioFrame *>(20);
    videoQueue = new IAVBlockQueue<VideoFrame *>(20);


    audioPlayer = new OpenSLESPlayer();
    if(!audioPlayer->create())
    {
        LOGE("audio player create failed");
    } else{
        LOGE("audio player created");
        audioPlayer->setAudioFrameProvider(this);
    }
    videoPlayer = new IVideoPlayer();

    exitFlag = false;
}

MediaPlayerController::~MediaPlayerController() {
    exitFlag = true;

    if(decoder != nullptr)
    {
        closeFile();
        delete(decoder);
        decoder = nullptr;
    }
    if(audioPlayer != nullptr)
    {
        audioQueue->notifyWaitGet();
        delete(audioPlayer);
        audioPlayer = nullptr;
    }
    if(videoPlayer != nullptr)
    {
        videoQueue->notifyWaitGet();
        delete(videoPlayer);
        videoPlayer = nullptr;
    }


    if(audioQueue != nullptr)
    {
        audioQueue->discardAll(nullptr);
        AudioFrame *frame = nullptr;
        while((frame = audioQueue->getUsed()) != nullptr)
        {
            delete(frame);
        }
    }


    if(videoQueue != nullptr)
    {
        videoQueue->discardAll(nullptr);
        VideoFrame *frame = nullptr;
        while((frame = videoQueue->getUsed()) != nullptr)
        {
            delete(frame);
        }
    }

    if(nextVFrame != nullptr)
    {
        delete(nextVFrame);
    }
    if(waitVFrame != nullptr)
    {
        delete(waitVFrame);
    }

}

bool MediaPlayerController::openFile(const char *path) {
    LOGD("openFile");
    if(!decoder->openFile(path))
    {
        LOGE("decoder open file failed");
        return false;
    }
    return true;
}

void MediaPlayerController::closeFile() {
    LOGD("closeFile");
    audioQueue->discardAll(nullptr);
    videoQueue->discardAll(nullptr);
    decoder->closeInput();
    LOGD("decoder close input finished");
    audioQueue->discardAll(nullptr);
    videoQueue->discardAll(nullptr);
    videoQueue->notifyWaitGet();
    audioQueue->notifyWaitGet();
    LOGD("closeFile exit");
}

void MediaPlayerController::start() {
    LOGD("audio start");
    audioPlayer->start();
}

void MediaPlayerController::stop() {
    LOGD("stop");
    audioPlayer->stop();
}

void MediaPlayerController::discardAllFrame() {
    videoQueue->discardAll(nullptr);
    audioQueue->discardAll(nullptr);
}

void MediaPlayerController::setWindow(ANativeWindow *window) {
    if(!videoPlayer){
        LOGD("setWindow : videoPlayer is nullptr");
        return;
    }
    videoPlayer->setWindow(window);
}

void MediaPlayerController::setSize(int width, int height) {
    if(!videoPlayer){
        LOGD("setSize : videoPlayer is nullptr");
        return;
    }
    videoPlayer->setSize(width, height);
}

int64_t MediaPlayerController::getDuration() {
    return decoder->getDuration();
}

void MediaPlayerController::receiveAudioFrame(AudioFrame *audioData){
    audioQueue->put(audioData);
}

void MediaPlayerController::receiveVideoFrame(VideoFrame *videoData) {
    videoQueue->put(videoData);
}

AudioFrame *MediaPlayerController::getUsedAudioFrame(){

    return audioQueue->getUsed();
}

VideoFrame *MediaPlayerController::getUsedVideoFrame() {
    return videoQueue->getUsed();
}

void MediaPlayerController::putUsedAudioFrame(AudioFrame *audioData) {
    audioQueue->putToUsed(audioData);
}

void MediaPlayerController::putUsedVideoFrame(VideoFrame *videoData) {
    videoQueue->putToUsed(videoData);
}

AudioFrame *MediaPlayerController::getAudioFrame() {
    LOGD("audio frame,");
    AudioFrame *frame = audioQueue->get();
    if(!frame)
    {
        //播放完成
        LOGD("audio play end");
        return nullptr;
    }
    currentPositionMS = frame->pts;
    LOGV("audio frame, currentPositionMS = %ld", currentPositionMS);
    while(1)
    {
        if(waitVFrame == nullptr)
        {
            waitVFrame = videoQueue->get();
            if(waitVFrame == nullptr)
            {
                break;
            }

            LOGV("audio frame, pts = %ld, videoQueue.size = %d", waitVFrame->pts, videoQueue->getSize());
        }

        LOGV("audio frame, pts = %ld", waitVFrame->pts);

        if(waitVFrame->pts - currentPositionMS < -50)
        {
            ///this video too late, discard it. And continue this loop until get a AVFrame is not too late
            LOGV("audio frame video too late, discard");
            videoQueue->putToUsed(waitVFrame);
            waitVFrame = nullptr;
            continue;
        } else if(waitVFrame->pts - currentPositionMS > 50)
        {
            //this video is still too early, wait. break this loop to let the AVFrame return. And will check this video frame at next getAVFrame call.
            LOGV("audio frame video too early, wait. video is early %ld ms than audio", waitVFrame->pts - frame->pts);
            break;
        } else
        {
            ///it is time to refresh this AVFrame.
            if(videoPlayer->isReady())
            {
                LOGV("audio frame :refresh image");
                unique_lock<mutex> AVFrameLock(videoMu);
                nextVFrame = waitVFrame;
                waitVFrame = nullptr;
                LOGV("audio frame : nextVFrame: w %d ,h %d",nextVFrame->width,nextVFrame->height);
                videoPlayer->renderSurface(nextVFrame->data);
                videoQueue->putToUsed(nextVFrame);
                AVFrameLock.unlock();
            } else
            {
                ///videoPlayer not ready(caused window not set), discard this video frame
                LOGE("audio frame ,videoPlayer not prepared, discard this AVFrame");
                videoQueue->putToUsed(waitVFrame);
                waitVFrame = nullptr;
            }
            break;
        }
    }
    return frame;
}

void MediaPlayerController::putBackUsed(AudioFrame *data) {
    audioQueue->putToUsed(data);
}

VideoFrame *MediaPlayerController::getVideoFrame() {
    LOGV("audio frame videoPlayer call getVFrame");
    unique_lock<mutex> locker(videoMu);
    LOGV("audio frame videoPlayer call getVFrame1");
    VideoFrame *f = nextVFrame;
    nextVFrame = nullptr;
    locker.unlock();
    return f;
}

void MediaPlayerController::putBackUsed(VideoFrame *data) {
    videoQueue->putToUsed(data);
}