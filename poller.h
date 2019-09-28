#ifndef _SEENET_POLLER_H
#define _SEENET_POLLER_H

#include "noncopyable.h"
#include "seenet.h"
#include "eventloop.h"

#include <memory>
#include <vector>
#include <ctime>
#include <map>
namespace seenet{
    namespace net {
        class Channel;
        class EventLoop;

        class Poller: public NonCopyable
        {
        public:
          using ChannelArray=std::vector<Channel*>;
          using ChannelMap = std::map<int, Channel*>; 
          Poller(EventLoop_sPt loop);
          virtual ~Poller();
          virtual std::time_t poll(int timeoutMs, ChannelArray* activeChannels){};
          virtual void updateChannel(Channel_sPt channel){};
          virtual void removeChannel(Channel_sPt channel){};
          virtual bool hasChannel(Channel_sPt channel);
          static Poller* newDefaultPoller(EventLoop_sPt loop);
          void assertInLoopThread()
          {
            m_loop->assertInLoopThread();
          };
        private:
          EventLoop_sPt m_loop;
        protected:
          ChannelMap  m_channels;
        };
    }
}
#endif