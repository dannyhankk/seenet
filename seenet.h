#ifndef _SEENET_H
#define _SEENET_H

#define __x86_64__
#include<memory>
#include<functional>

namespace seenet{
    namespace net {
        class Channel;
        class EventLoop;
        class Timer;
        class TcpConnection;
        class Buffer;
        

        //shared_ptrs
        using EventLoop_sPt=std::shared_ptr<EventLoop>;
        using Channel_sPt=std::shared_ptr<Channel>;
        using Timer_sPt=std::shared_ptr<Timer>;
        using TcpConnection_sPt=std::shared_ptr<TcpConnection>;

        // weak_ptrs
        using EventLoop_wPt=std::weak_ptr<EventLoop>;
        using Channel_wPt=std::weak_ptr<Channel>;
        using Timer_wPt=std::weak_ptr<Timer>;
        using TcpConnection_wPT=std::weak_ptr<TcpConnection>;
        
        // callbacks
        using TimerCallback=std::function<void()>;
        using ConnectionCallback=std::function<void(const TcpConnection_sPt&)>;
        using CloseCallback=std::function<void(const TcpConnection_sPt)>;
        using WriteCompleteCallback=std::function<void(const TcpConnection_sPt)>;
        using HighWaterMarkCallback=std::function<void(const TcpConnection_sPt, size_t)>;

        using MessageCallback = std::function<void(const TcpConnection_sPt&, Buffer*, std::time_t)>;



    }
}
#endif