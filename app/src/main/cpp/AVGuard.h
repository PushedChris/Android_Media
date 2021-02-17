//
// Created by comochirs on 2/17/21.
//

#ifndef MEDIAPLAYER_AVGUARD_H
#define MEDIAPLAYER_AVGUARD_H

#include "IAVDecoder.h"

template<typename T>
class AVGuard {
public:
    AVGuard();
    ~AVGuard();
    AVGuard(T* _ctx);
    T *get(){
        return m_ctx;
    }
    T operator *()const {
        return *m_ctx;
    }
    T* operator ->(){
        return m_ctx;
    }
    T ** operator &(){
        return &m_ctx;
    }
    //重载复制操作
    T* operator = (T* _ctx){
        m_ctx = _ctx;
        return m_ctx;
    }
private:
    T *m_ctx;
};

template <typename T>
AVGuard<T>::AVGuard(){
    av_log(NULL, AV_LOG_INFO, "AVGuard：Definition Construction\n");
}

template<typename T>
AVGuard<T>::AVGuard(T* _ctx){
    m_ctx = _ctx;
}

template <typename T>
AVGuard<T>::~AVGuard(){
    av_log(NULL, AV_LOG_INFO, "AVGuard：Definition Deconstruction\n");
    if (m_ctx) {
        m_ctx = nullptr;
    }
}


#endif //MEDIAPLAYER_AVGUARD_H
