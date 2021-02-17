### 基础知识
1. 编码
2. 格式
3. 流
4. 帧
5. 基础命名规则
### 源码架构分析
##### 源码大纲
在做项目之前，先对源码结构和应用方式进行一定的了解，
[雷霄骅](https://blog.csdn.net/leixiaohua1020)博士总结综合了使用方式和源码结构，这样理解起来十分清晰，本人做了一些标注方便后面再看：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210214152634693.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzMzMjcxMTky,size_16,color_FFFFFF,t_70)
##### 核心模块
1. libavformat                               （格式封装）
    1. Demuxer                              （解封装）
    2. Muxer                                   （封装）
2. libavcodec                                （编解码）
     1. Parser                                 （解析器）
         将零散的流转换为单独的帧
     3. Decoder                              （解码器）
     4. Encoder                              （编码器）
3. libavutil                                     
    1. pixel                                     （像素表示）
    2. Timestamp                             (时间表示)
    3. IO/Buffer                              （流处理工具）

##### Android 视频播放器
考虑写一个最简单的视频播放器，大致逻辑如下：
PS: 为了使代码更加直观，下文中代码没有错误处理和日志，并且将内存申请使用RAII封装至另外一个类中。
```cpp
    //1. 创建封装格式上下文
    AVGuard<AVFormatContext> fmt_ctx;

    //2. 打开输入文件，解封装
    err = avformat_open_input(&fmt_ctx, TEST_FILE_TFCARD, NULL, NULL);
    //获取音视频流信息
   err = avformat_find_stream_info(fmt_ctx.get(), NULL)；
    //获取音视频流引索
    // search video stream in all streams.
    for (i = 0; i < fmt_ctx->nb_streams; i++) {
        // because video stream only one, so found and stop.
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    //5. 获取解码器参数
    AVGuard<AVCodecParameters> codev_param = fmt_ctx->streams[video_stream_index]->codecpar;

    //6. 根据codeid获取解码器
    AVGuard<AVCodec> vcodec = avcodec_find_decoder(codev_param->codec_id);

    //7. 创建解码器上下文
   AVGuard<AVCodecContext> vcodec_ctx(avcodec_alloc_context3(vcodec.get()));
    avcodec_parameters_to_context(vcodec_ctx.get(),codev_param.get()

    //8. 打开解编码器
    avcodec_open2(vcodec_ctx.get(), vcodec.get(),
            NULL)；

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
                av_log(NULL, AV_LOG_ERROR, "get frame : %d. \n",m_StreamIndex);
                AVFrame pict;
                av_image_alloc(pict.data, pict.linesize,
                               vcodec_ctx->width,
                               vcodec_ctx->height, AV_PIX_FMT_RGB565LE, 16);

                // Convert the image into YUV format that SDL uses
                img_convert(&pict, AV_PIX_FMT_RGB565LE,m_Frame.get(),
                            vcodec_ctx->pix_fmt,
                            vcodec_ctx->width,
                            vcodec_ctx->height);
                            
                //将RGB数据渲染至ANativeWindow中
                renderSurface(pict.data[0]);

                av_freep(&pict.data[0]);
            }
        }
        av_packet_unref(m_Packet.get()); //释放 m_Packet 引用，防止内存泄漏
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
```
### 问题记录
记录一下遇到的小问题：
##### 新旧版本API问题
    1. 弃用av_register_all
    2. 弃用AVStream::codec
    3. 弃用avcodec_decode_video2
    4. 弃用AVPictrue
##### 对Android媒体读写权限问题
    使用Android 9.0+（29+）版本的API没有对本地文件的读写权限，每次返回-13（permission denied）将版本退回至28，可以暂时解决问题
