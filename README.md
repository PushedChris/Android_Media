### OpenSL基础
OpenSLES接口可以直接载Native层处理音频数据，减少了Java层到Native层在采集、播放和编解码过程中的数据拷贝。
OpenSLES文档：OpenSL_ES_Specification1.1.pdf
##### 优点
1. C语言接口，使用NDK，翻边深度优化，例如NEON优化。
2. 没有垃圾回收机制，需要自己实现垃圾回收
3. 支持PCM数据采集
4. 支持PCM数据的播放
##### Objects 和 Interfaces
1. 官方为每一种Objects都定义了一系列Interface
2. 先通过GetInterface拿到接口，再通过Interface访问功能函数
3. 通过Interface ID可以获取对应的功能接口
##### 状态机制
文档中状态机制图：
![在这里插入图片描述](https://img-blog.csdnimg.cn/20210219232343208.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzMzMjcxMTky,size_16,color_FFFFFF,t_70)


每个对象都有一些最基础的方法：Realize、Resume、GetState、Destory
如上图所示，只有对象realize之后系统才会正确得分配资源。
##### 常用结构体
1. Engine Object 和 SLEngineltf（管理接口，用于创建其他对象）
2. Media Object（多媒体抽象功能）
3. Data Source 和Data Sink（输入源和输出源）
### FFmpeg解码音频数据![在这里插入图片描述](https://img-blog.csdnimg.cn/20210219233728351.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzMzMjcxMTky,size_16,color_FFFFFF,t_70)
一般的处理方式为：利用 FFmpeg 对一个 Mp4 文件的音频流进行解码，然后使用 libswresample 将解码后的 PCM 音频数据转换为目标格式的数据，重采样来确保音频采样率和设备驱动采样率一致，使音频正确播放。最后利用 OpenSLES 进行播放。

播放源代码：
```cpp
 //创建引擎对象
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    //初始化引擎对象
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    //创建并初始化混音器
    SLInterfaceID ids1[1] = {SL_IID_OUTPUTMIX};
    SLboolean reqs1[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, ids1, reqs1);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    // 创建并初始化播放器
    SLDataLocator_AndroidSimpleBufferQueue bufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcmFormat = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&bufferQueue, &pcmFormat};

    SLDataLocator_OutputMix locOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&locOutputMix, NULL};

    SLInterfaceID ids2[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    SLboolean reqs2[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSink, 2, ids2, reqs2);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, audioCallback, this);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

//    result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &bqPlayerVolume);
//    if(result != SL_RESULT_SUCCESS)
//    {
//        return false;
//    }

    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PAUSED);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }

    (*playerBufferQueue)->Enqueue(playerBufferQueue, emptyBuffer, EMPTY_BUFFER_SAMPLES * 2 * sizeof(int16_t));

    return true;
```
`RegisterCallback(playerBufferQueue, audioCallback, this);`用来注册回调函数，回调函数原理：
```cpp
void OpenSLESPlayer::processAudio() {
    //注意这个是另外一条采集线程回调，
    unique_lock<mutex> providerLock(providerMu);
    if(provider != NULL)
    {
        AudioFrame *data = provider->getAudioFrame();
        if(data == NULL)
        {
            LOGE("get a NULL audio frame");
            (*playerBufferQueue)->Enqueue(playerBufferQueue, emptyBuffer, EMPTY_BUFFER_SAMPLES * 2 * sizeof(int16_t));
        } else
        {
//            LOGD("audio frame sample count = %d, pts = %ld", data->sampleCount, data->pts);
            memcpy(emptyBuffer, data->data, data->sampleCount * 2 * sizeof(int16_t));
            // 取完数据，需要调用Enqueue触发下一次数据回调
            (*playerBufferQueue)->Enqueue(playerBufferQueue, emptyBuffer, data->sampleCount * 2 * sizeof(int16_t));
            provider->putBackUsed(data);
        }
    }
    providerLock.unlock();
}
```
>Github:[https://github.com/PushedChris/Android_Media/tree/player_OpenSL_Audio](https://github.com/PushedChris/Android_Media/tree/player_OpenSL_Audio)
>CSDN:[https://blog.csdn.net/qq_33271192/article/details/113873898](https://blog.csdn.net/qq_33271192/article/details/113873898)
