#include "../poller.h"
#include "epollpoller.h"
#include "../eventloop.h"

namespace seenet{
    namespace net{
        Poller* Poller::newDefaultPoller(EventLoop_sPt loop)
        {
             return new EPollPoller(loop);
        }
    }
}