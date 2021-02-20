
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <assert.h>
#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <android/log.h>
#include <time.h>
#include <utime.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sched.h>

#include "IAVBlockQueue.h"
#include "AVQueueInterface.h"

extern "C" {
#include "libavutil/log.h"
#include "libavutil/time.h"
#include "libavutil/samplefmt.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include <libavutil/imgutils.h>
}

#include <android/log.h>


using namespace std;

class IAVDecoder{
public:
    IAVDecoder();
    ~IAVDecoder();

    bool openFile(const char *inputFile);
    void closeInput();
    bool hasAudio();
    bool hasVideo();

    void setDataReceiver(AVQueueInterface* _dataReceiver);

    int32_t getVideoWidth();
    int32_t getVideoHeight();

    int64_t getDuration();

private:
    bool initComponents(const char *path);
    void resetComponents();

    void decodeAudio();
    void decodeVideo();
    void readFile();

    AVPacket *getFreePacket();

    static void audioThreadCallback(void *context);
    static void videoThreadCallback(void *context);
    static void readThreadCallback(void *context);

    void recyclePackets();

    void discardAllReadPackets();

    AVFormatContext *formatCtx = NULL;

    int audioIndex = -1, videoIndex = -1;
    AVStream *audioStream = NULL, *videoStream = NULL;
    AVCodecContext *audioCodecCtx = NULL, *videoCodecCtx = NULL;
    AVCodec *audioCodec, *videoCodec = NULL;
    SwrContext *audioSwrCtx = NULL;
    SwsContext *videoSwsCtx = NULL;

    int64_t duration = 0;

    thread *audioDecodeThread = NULL;
    thread *videoDecodeThread = NULL;
    thread *readThread = NULL;

    bool stopDecodeFlag = false;

    float videoFPS = 0;

    int32_t audioSampleCountLimit = 0;

    static const int32_t AUDIO_SAMPLE_RATE = 44100;

    bool audioDecodeFinished = false;
    bool videoDecodeFinished = false;

    IAVBlockQueue<AVPacket *> *audioPacketQueue;
    IAVBlockQueue<AVPacket *> *videoPacketQueue;

    AVQueueInterface *dataReceiver;

    mutex componentsMu;

//    int mPlayerWidth = 1920;
//    int mPlayerHeight = 1080;
};



#define TAG "FFmpeg"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

#endif /* __PLAYER_H__ */