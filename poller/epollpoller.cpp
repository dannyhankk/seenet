
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "epollpoller.h"

#include "../seenet.h"
#include "../channel.h"

#include <assert.h>
#include <sys/errno.h>
#include <strings.h>

namespace{
    // a new channel
    const int kNew = -1;
    const int kAdded = 1;
    // an reused channel
    const int kDeleted = 2;
}

namespace seenet{
    namespace net
    {
        EPollPoller::EPollPoller(EventLoop_sPt loop):Poller(loop),
                      m_epollFd(::epoll_create1(EPOLL_CLOEXEC)),
                      m_events(m_InitEeventArraySize)
        {
              if(m_epollFd < 0)
              {
                  //todo 
                  // fatal error
              }
        }

        EPollPoller::~EPollPoller()
        {
            ::close(m_epollFd);
        }

        std::time_t EPollPoller::poll(int timeoutMs, ChannelArray* activeChannelList)
        {
            int numEvents = ::epoll_wait(m_epollFd,
                                         &*m_events.begin(),
                                         static_cast<int>(m_events.size()),
                                         timeoutMs);
            int saveErrno = errno;
            std::time_t now(std::time(nullptr));
            if(numEvents > 0)
            {
                fillActiveChannels(numEvents, activeChannelList);
                if(static_cast<size_t>(numEvents) == m_events.size())
                {
                    //resize double
                    m_events.resize(m_events.size() * 2);
                }
            }
            else if(numEvents == 0)
            {
                // time-out do nothing
            }
            else 
            {
                if(saveErrno != EINTR)
                {
                    errno = saveErrno;
                    // log fatal error
                }
            }
            return now;
        }// poll

        void EPollPoller::fillActiveChannels(int numEvents,
                                        ChannelArray* activeChannels) const
        {
           assert(static_cast<size_t>(numEvents) <= m_events.size());
           for(int i = 0; i < numEvents; i ++)
           {
               Channel* ch = static_cast<Channel*>(m_events[i].data.ptr);
#ifndef NDEBUG
               int fd = ch->fd();
               ChannelMap::const_iterator it = m_channels.find(fd);
               assert(it != m_channels.end());
               assert(it->second == ch);
#endif
               //give events to channel
               ch->set_revents(m_events[i].events);
               // en-queue channel
               activeChannels->push_back(ch);
           } //for
        }// fillActiveChannels

        void EPollPoller::updateChannel(Channel_sPt channel)
        {
            Poller::assertInLoopThread();
            const int index = channel->index();

            // a new channel, add with EPOLL_CTL_ADD
            if(index == kNew || index == kDeleted)
            {
                int fd = channel->fd();
                if(index == kNew)
                {
                    assert(m_channels.find(fd) == m_channels.end());
                    m_channels[fd] = channel.get();
                }
                else // index == kDeleted
                {
                    assert(m_channels.find(fd) != m_channels.end());
                    assert(m_channels[fd] == channel.get());
                }

                channel->set_idnex(kAdded);
                update(EPOLL_CTL_ADD, channel);
            }
            else
            {
                // update existing one with EPOLL_CTL_MOD/DEL
                int fd = channel->fd();
                assert(m_channels.find(fd) != m_channels.end());
                assert(m_channels[fd] == channel.get());
                if(channel->isNonEvent())
                {
                    update(EPOLL_CTL_DEL, channel);
                    channel->set_idnex(kDeleted);
                }
                else
                {
                    update(EPOLL_CTL_MOD, channel);
                } 
            }
            
        } // updatechannel

        void EPollPoller::removeChannel(Channel_sPt channel)
        {
            Poller::assertInLoopThread();
            int fd = channel->fd();
            //todo: log fd num
            assert(m_channels.find(fd) != m_channels.end());
            assert(m_channels[fd] == channel.get());
            assert(channel->isNonEvent());
            int index = channel->index();
            assert(index == kAdded || index == kDeleted);
            size_t n = m_channels.erase(fd);
            (void)n; // in case warning of unused variable
            assert(n == 1);

            if(index == kAdded)
            {
                update(EPOLL_CTL_DEL, channel);
            }
            channel->set_idnex(kNew);
        } // removeChannel

        void EPollPoller::update(int operation, Channel_sPt channel)
        {
            struct epoll_event event;
            bzero(&event, sizeof(event));
            event.events = channel->events();
            event.data.ptr = channel.get();

            int fd = channel->fd();

            if(::epoll_ctl(m_epollFd, operation, fd, &event) < 0)
            {
                if(operation == EPOLL_CTL_DEL)
                {
                    //todo log
                }
                else
                {
                    //todo log
                }
            }
        } // update

        const char *EPollPoller::operationToString(int op)
        {
            switch(op)
            {
                case EPOLL_CTL_ADD:
                    return "ADD";
                case EPOLL_CTL_DEL:
                    return "DEL";
                case EPOLL_CTL_MOD:
                    return "MOD";
                default:
                    assert(false && "ERROR op");
                    return "Unknown Operation";
            }
        }
    } // namespace net
    
}