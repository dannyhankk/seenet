#include "seenet.h"
#include "eventloop.h"
#include "poller.h"
#include "channel.h"

#include<assert.h>

namespace{
    const int kPollTimeMs = 10000;
}

namespace seenet{
    namespace net {
        EventLoop::EventLoop():
        m_bLooping(false),
        m_threadId(std::this_thread::get_id())
        {
            
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
           while(!m_bLooping.load())
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
           }

           m_bLooping.store(false);
        }// <<end loop>>

        void EventLoop::quit()
        {
            m_bLooping.store(false);
            if(!isInLoopThread)
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

        void EventLoop::quit()
        {
            m_bLooping.store(false);
            wakeup();
        }

        void EventLoop::updateChannel(Channel_sPt channel)
        {
            assert(channel->ownerLoop().get() == this);
            m_poller->updateChannel(channel);
        }

        void EventLoop::removeChannel(Channel_sPt channel)
        {
            assert(channel->ownerLoop().get() == this);
            assertInLoopThread();
            
            m_poller->removeChannel(channel);
        }

        bool EventLoop::hasChannel(Channel_sPt channel)
        {
            assert(channel->ownerLoop().get() == this);
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
    }
}