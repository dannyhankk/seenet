#include "seenet.h"
#include "eventloop.h"
#include "poller.h"
#include "channel.h"
#include "./util/util_socketops.h"
#include "timerqueue.h"

#include <assert.h>
#include <unistd.h>
#include <iostream>

#include<signal.h>
#include<sys/eventfd.h>


namespace{
    const int kPollTimeMs = 10000;
}

namespace seenet{
    namespace net {

            namespace{
           int createEventFd()
           {
               int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
               if(evtfd < 0)
               {
                   //log sys
                   abort();
               }
               return evtfd;
           } 
        }
        EventLoop::EventLoop():m_bLooping(false),
        m_threadId(std::this_thread::get_id()),
        m_bCallingPendingFactors(false),
        m_wakeupFd(createEventFd()),
        m_poller(Poller::newDefaultPoller(this)),
        m_wakeupChannel(new Channel(this, m_wakeupFd)),
        m_currentActiveChannel(NULL), 
        m_timerQueue(new TimerQueue(this))
        {
            m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
            m_wakeupChannel->enableReading();
        }

        EventLoop::~EventLoop()
        {
            // do nothing
        }
        void EventLoop::loop()
        {
           assert(!m_bLooping.load());
           m_bLooping.store(true);

           //log
           while(m_bLooping.load())
           {
                m_activeChannels.clear();
                m_PollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);

                for(ChannelArray::iterator it = m_activeChannels.begin(); it != m_activeChannels.end(); it ++)
                {
                    m_currentActiveChannel = *it;
                    m_currentActiveChannel->handleEvent();
                }
                m_currentActiveChannel = NULL;
                doPendingFunctors();
                std::cout<<"one round loop" << std::endl;
           }

           m_bLooping.store(false);
        }// <<end loop>>

        void EventLoop::handleRead()
        {
            uint64_t one = 1;
            ssize_t n = sockets::read(m_wakeupFd, &one, sizeof(one));
            if(n != sizeof(one))
            {
                //log sys
            }
        }

        void EventLoop::quit()
        {
            m_bLooping.store(false);
            if(!isInLoopThread())
            {
                wakeup();
            }
        }

        void EventLoop::doPendingFunctors()
        {
            std::vector<Functor> functors;
            m_bCallingPendingFactors = true;

            {
                std::lock_guard<std::mutex> lock(m_pendingFactorLock);
                functors.swap(m_pendingFunctors);
            }

            for(size_t i = 0; i < functors.size(); i ++)
            {
                functors[i]();
            }

            m_bCallingPendingFactors = false;
        }// end doPendingFunctors

        void EventLoop::updateChannel(Channel_sPt channel)
        {
            assert(channel->ownerLoop() == this);
            m_poller->updateChannel(channel);
        }

        void EventLoop::removeChannel(Channel_sPt channel)
        {
            assert(channel->ownerLoop() == this);
            assertInLoopThread();
            
            m_poller->removeChannel(channel);
        }

        bool EventLoop::hasChannel(Channel_sPt channel)
        {
            assert(channel->ownerLoop() == this);
            assertInLoopThread();
            return m_poller->hasChannel(channel);
        }

        void EventLoop::abortNotInLoopThread()
        {
            // abort
        }

        void EventLoop::runInLoop(const Functor& cb)
        {
            if(isInLoopThread())
            {
                cb();
            }
            else
            {
                queueInLoop(cb);
            }
        }

        void EventLoop::queueInLoop(const Functor& cb)
        {
            {
                std::lock_guard<std::mutex> lock(m_pendingFactorLock);
                m_pendingFunctors.push_back(cb);
            }

            if(isInLoopThread() || m_bCallingPendingFactors)
            {
                wakeup();
            }
        }

        size_t EventLoop::queueSize() const
        {
            std::lock_guard<std::mutex> lock(m_pendingFactorLock);
            return m_pendingFunctors.size();
        }

        void EventLoop::wakeup()
        {
            uint16_t one = 1;
            ssize_t n = sockets::write(m_wakeupFd, &one, sizeof(one));
            if(n != sizeof(one))
            {
                //log sys
            }
        }

        TimerId EventLoop::runAt(const std::time_t& time, const TimerCallback& cb)
        {
            return m_timerQueue->addTimer(std::move(cb), time, 0.0);
        }

        TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
        {
            std::time_t time(std::time(nullptr));
            
            time += delay; // fix to add time properly

            return runAt(time, std::move(cb));
        }


    }
}