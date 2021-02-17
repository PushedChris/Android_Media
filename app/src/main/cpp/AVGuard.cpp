//
// Created by comochirs on 2/17/21.
//
#include "AVGuard.h"

//模版特化的安全退出函数
//AVFormateContext
template <>
AVGuard<AVFormatContext>::AVGuard(){
    av_log(NULL, AV_LOG_INFO, "AVGuard：AVFormatContext Definition Construction\n");
    m_ctx = avformat_alloc_context();
}
template <>
AVGuard<AVFormatContext>::~AVGuard(){
    if (m_ctx) {
        av_log(NULL, AV_LOG_INFO, "AVGuard：AVFormatContext Definition Deconstruction\n");
        avformat_close_input(&m_ctx);
        avformat_free_context(m_ctx);
    }
    m_ctx = nullptr;
}

//AVPacket
template <>
AVGuard<AVPacket>::AVGuard(){
    m_ctx = av_packet_alloc();
}
template <>
AVGuard<AVPacket>::~AVGuard(){
    AVPacket *m_avpacket = m_ctx;
    if (m_avpacket) {
        av_packet_free(&m_avpacket);
        m_avpacket = nullptr;
    }
}

//AVFrame
template <>
AVGuard<AVFrame>::AVGuard(){
    m_ctx = av_frame_alloc();
}
template <>
AVGuard<AVFrame>::~AVGuard(){
    AVFrame *m_avframe = m_ctx;
    if (m_avframe) {
        av_frame_free(&m_avframe);
        m_avframe = nullptr;
    }
}

//AVCodecContext
template<>
AVGuard<AVCodecContext>::~AVGuard() {
    if(m_ctx){
        avcodec_close(m_ctx);
        avcodec_free_context(&m_ctx);
        m_ctx = nullptr;
    }
}