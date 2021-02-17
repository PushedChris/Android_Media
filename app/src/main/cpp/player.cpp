#include <stdio.h>
#include <signal.h>

#include "player.h"
#include "AVGuard.h"

#define TEST_FILE_TFCARD "/storage/emulated/0/DCIM/Camera/abc.mp4"
GlobalContext global_context;

static void ffmpeg_log_callback(void *ptr, int level, const char *fmt,
                                va_list vl) {
    __android_log_vprint(ANDROID_LOG_DEBUG, "FFmpeg", fmt, vl);
}

static int img_convert(AVFrame *dst, int dst_pix_fmt, const AVFrame *src,
                       int src_pix_fmt, int src_width, int src_height) {
    int w;
    int h;
    struct SwsContext *pSwsCtx;

    w = src_width;
    h = src_height;

    pSwsCtx = sws_getContext(w, h, (AVPixelFormat) src_pix_fmt, w, h,
                             (AVPixelFormat) dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
    sws_scale(pSwsCtx, (const uint8_t* const *) src->data, src->linesize, 0, h,
              dst->data, dst->linesize);

    return 0;
}

void* open_media(void *argv) {
    int i;
    int err = 0;

    AVPacket pkt;
    int video_stream_index = -1;

    av_log_set_callback(ffmpeg_log_callback);
    // set log level
    av_log_set_level(AV_LOG_INFO);

    //1. 创建封装格式上下文
    AVGuard<AVFormatContext> fmt_ctx;

    //2. 打开输入文件，解封装
    err = avformat_open_input(&fmt_ctx, TEST_FILE_TFCARD, NULL, NULL);
    if (err < 0) {
        char errbuf[64];
        av_strerror(err, errbuf, 64);
        av_log(NULL, AV_LOG_ERROR, "avformat_open_input : err is %d , %s\n",
               err, errbuf);
        err = -1;
        return 0;
    }
    //获取音视频流信息
    if ((err = avformat_find_stream_info(fmt_ctx.get(), NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info : err is %d \n",
               err);
        err = -1;
        return 0;
    }

    //获取音视频流引索
    // search video stream in all streams.
    for (i = 0; i < fmt_ctx->nb_streams; i++) {
        // because video stream only one, so found and stop.
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    // if no video and audio, exit
    if (-1 == video_stream_index) {
        return 0;
    }


    //5. 获取解码器参数
    AVGuard<AVCodecParameters> codev_param = fmt_ctx->streams[video_stream_index]->codecpar;

    //6. 根据codeid获取解码器
    AVGuard<AVCodec> vcodec = avcodec_find_decoder(codev_param->codec_id);
    if (!vcodec.get()) {
        av_log(NULL, AV_LOG_ERROR,
               "avcodec_find_decoder video failure. \n");
        return 0;
    }

    //7. 创建解码器上下文
    AVGuard<AVCodecContext> vcodec_ctx(avcodec_alloc_context3(vcodec.get()));
    if(avcodec_parameters_to_context(vcodec_ctx.get(),codev_param.get()) != 0){
        av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failure. \n");
        return 0;
    }

    //8. 打开解编码器
    if (avcodec_open2(vcodec_ctx.get(), vcodec.get(),
            NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failure. \n");
        return 0;
    }

    if ((vcodec_ctx->width > 0)&& (vcodec_ctx->height > 0)) {
        setBuffersGeometry(vcodec_ctx->width,
                vcodec_ctx->height);
    }
    av_log(NULL, AV_LOG_ERROR, "video : width is %d, height is %d . \n",
            vcodec_ctx->width,
            vcodec_ctx->height);


    //9. 创建编码数据和解码数据结构体
    AVGuard<AVPacket> m_Packet;
    AVGuard<AVFrame> m_Frame;
    int m_StreamIndex = 0;

    //10. 解码循环
    while (av_read_frame(fmt_ctx.get(), m_Packet.get()) >= 0) {
        if (m_Packet->stream_index == m_StreamIndex) {
            if (avcodec_send_packet(vcodec_ctx.get(), m_Packet.get()) != 0) {
                return 0;
            }
            while (avcodec_receive_frame(vcodec_ctx.get(), m_Frame.get()) == 0) {
                //获取到 m_Frame 解码数据，进行格式转换，然后进行渲染
                av_log(NULL, AV_LOG_ERROR, "get frame : %d. \n", m_StreamIndex);
                AVFrame pict;
                av_image_alloc(pict.data, pict.linesize,
                               vcodec_ctx->width,
                               vcodec_ctx->height, AV_PIX_FMT_RGB565LE, 16);

                // Convert the image into YUV format that SDL uses
                img_convert(&pict, AV_PIX_FMT_RGB565LE, m_Frame.get(),
                            vcodec_ctx->pix_fmt,
                            vcodec_ctx->width,
                            vcodec_ctx->height);

                renderSurface(pict.data[0]);

                av_freep(&pict.data[0]);
            }
        }
        av_packet_unref(m_Packet.get()); //释放 m_Packet 引用，防止内存泄漏
    }

    return 0;
}

