#include "seenet.h"
#include "channel.h"
#include "EventLoop.h"

#include <sys/epoll.h>
#include <unistd.h>

#include <memory.h>

namespace seenet{
    namespace net {

        const int Channel::kNoneEvent = 0;
        const int Channel::kReadEvent = EPOLLIN | EPOLLIN;
        const int Channel::kWriteEvent = EPOLLOUT;
        /*
            channel used to hold an fd and do some 
          operations on this fd; 
            one channel for one fd only;
        */
        Channel::Channel(EventLoop_sPt loop, int fd):
            m_fd(fd),
            m_loop(loop),
            m_events(0),
            m_revents(0),
            m_index(-1)
        {

        }
        void Channel::update()
        {

            m_loop->updateChannel(shared_from_this());   
        }

        void Channel::handleEvent()
        {
            if(m_revents & EPOLLERR)
            {
                if(m_errorCallback)
                {
                    m_errorCallback();
                }
            }

            // read event
            if(m_revents & (EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP))
            {
               if(m_readCallback)
               {
                   m_readCallback(std::time(nullptr));
               }
            }
            // write event
            if(m_revents & (EPOLLOUT | EPOLLERR | EPOLLHUP))
            {
                if(m_writeCallback)
                {
                    m_writeCallback();
                }
            }  
        }

        void Channel::remove()
        {

            m_loop->removeChannel(shared_from_this());
        }
    }
}