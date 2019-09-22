#ifndef _SEENET_POLLER_H
#define _SEENET_POLLER_H

#include "noncopyable.h"
#include "seenet.h"

#include <memory>
#include <vector>
#include <ctime>
#include <map>
namespace seenet{
    namespace net {
        class Channel;
        class EventLoop;

        class Poller: public NonCopyable, std::enable_shared_from_this<Poller>
        {
        public:
          using ChannelArray=std::vector<Channel*>;
          using ChannelMap = std::map<int, Channel*>; 
          Poller(EventLoop_sPt loop);
          virtual ~Poller();
          virtual std::time_t poll(int timeoutMs, ChannelArray* activeChannels)= 0;
          virtual void updateChannel(Channel_sPt channel) = 0;
          virtual void removeChannel(Channel_sPt channel) = 0;
          virtual bool hasChannel(Channel_sPt channel);
          static Poller* newDefaultPoller(EventLoop_sPt loop);
          void assertInLoopThread();
        private:
          EventLoop_wPt m_loop;
        protected:
          ChannelMap  m_channels;
        };
    }
}
#endif