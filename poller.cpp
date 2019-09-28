#include "poller.h"
#include "channel.h"
#include "EventLoop.h"

namespace seenet{
    namespace net{
        Poller::Poller(EventLoop_sPt loop):
        m_loop(loop)
        {

        }

        Poller::~Poller() = default;
        bool Poller::hasChannel(Channel_sPt ch)
        {
            assertInLoopThread();
            ChannelMap::iterator it = m_channels.find(ch->fd());
            return it != m_channels.end() && it->second == ch.get();           
        }
    }
}