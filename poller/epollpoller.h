#ifndef _SEENET_EPOLL_H
#define _SEENET_EPOLL_H

#include "../noncopyable.h"
#include "../seenet.h"
#include "../poller.h"
#include <memory>
#include <vector>

/* IO Multiplexing with epoll(4) */
struct epoll_event;
namespace seenet{
    namespace net{
        class EPollPoller:public Poller, 
                          NonCopyable,
                          std::enable_shared_from_this<EPollPoller>
        {
        public:
            using EventArray = std::vector<struct epoll_event>;
            EPollPoller(EventLoop_sPt loop);
            ~EPollPoller();
            virtual std::time_t poll(int timeoutMs, ChannelArray* chList);
            virtual void updateChannel(Channel_sPt ch);
            virtual void removeChannel(Channel_sPt ch);
        private:

            //
            static const int m_InitEeventArraySize = 16;
            static const char *operationToString(int op);

            // inter epoll operation
            void fillActiveChannels(int numEvents,
                                    ChannelArray* activeChannels) const;
            void update(int operation, Channel_sPt channel);
            int m_epollFd;
            EventArray m_events;
        };
    }
}
#endif