#ifndef _SEENET_H
#define _SEENET_H

#ifndef __x86_64__
#define __x86_64__
#endif

#include<memory>
#include<functional>
#include<ctime>
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
        //for tcpconnection
        // reason use reference or shared_ptr
        // 回调函数不会做太多超时操作，都是在TcpConnection的线程中执行，这个指针也不太可能会被拷贝到其它地方使用
        // 因此可以使用引用，来减少一次指针复制
        using ConnectionCallback=std::function<void(const TcpConnection_sPt&)>;
        using CloseCallback=std::function<void(const TcpConnection_sPt&)>;
        using WriteCompleteCallback=std::function<void(const TcpConnection_sPt&)>;
        using HighWaterMarkCallback=std::function<void(const TcpConnection_sPt&, size_t)>;

        using MessageCallback = std::function<void(const TcpConnection_sPt&, Buffer*, std::time_t)>;


        //thread 
        using ThreadInitCallback = std::function<void(EventLoop_sPt)>;

        
        void defaultConnectionCallback(const TcpConnection_sPt& conn);
        void defaultMessageCallback(const TcpConnection_sPt& conn,
                                    Buffer* buffer,
                                    std::time_t receiveTime);



    }
}
#endif