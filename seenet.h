#ifndef _SEENET_H
#define _SEENET_H

#include<memory>

namespace seenet{
    namespace net {
        class Channel;
        class EventLoop;
        

        using EventLoop_sPt=std::shared_ptr<EventLoop>;
        using Channel_sPt=std::shared_ptr<Channel>;

        using EventLoop_wPt=std::weak_ptr<EventLoop>;
        using Channel_wPt=std::weak_ptr<Channel>;

    }
}
#endif