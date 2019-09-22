#ifndef _SEENET_H
#define _SEENET_H

#include<memory>

namespace seenet{
    namespace net {
        class Channel;
        class EventLoop;
        class Timer;
        

        //shared_ptrs
        using EventLoop_sPt=std::shared_ptr<EventLoop>;
        using Channel_sPt=std::shared_ptr<Channel>;
        using Timer_sPt=std::shared_ptr<Timer>;

        // weak_ptrs
        using EventLoop_wPt=std::weak_ptr<EventLoop>;
        using Channel_wPt=std::weak_ptr<Channel>;
        using Timer_wPt=std::weak_ptr<Timer>;
        
        // callbacks
        using TimerCallback=std::function<void()>;
    }
}
#endif