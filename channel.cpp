#include "seenet.h"
#include "channel.h"
#include "EventLoop.h"

#include <sys/epoll.h>
#include <unistd.h>

#include <memory.h>

namespace seenet{
    namespace net {
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
            auto sPt = m_loop.lock();
            if(sPt)
            {
                sPt->updateChannel(shared_from_this());
            }
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
                   m_readCallback();
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
            auto loop = m_loop.lock();
            if(loop)
            {
                loop->removeChannel(shared_from_this());
            }
        }
    }
}