#ifndef _SEENET_EVENTLOOP_H
#define _SEENET_EVENTLOOP_H
#include "seenet.h"
#include "noncopyable.h"

#include <thread>
#include <atomic>
#include <vector>
#include <ctime>
#include <memory>
#include <mutex>


namespace seenet{
    namespace net {
        class Channel;
        class Poller;
        class TimerQueue;

        class EventLoop: public NonCopyable,std::enable_shared_from_this<EventLoop>
        {
        public:
            using Functor = std::function<void()>;
            using ChannelArray=std::vector<Channel*>;
             EventLoop();
             ~EventLoop();

             void loop();
             void quit();
             void wakeup();
             void updateChannel(Channel_sPt ch);
             void removeChannel(Channel_sPt ch);

             bool hasChannel(Channel_sPt ch);

             void runInLoop(const Functor& cb);
             void queueInLoop(const Functor& cb);
             size_t queueSize() const;

             void assertInLoopThread()
             {
                 if(!isInLoopThread())
                 {
                     abortNotInLoopThread();
                 }
             }

             bool isInLoopThread()
             {
                 return m_threadId == std::this_thread::get_id();
             }

        private:
            void doPendingFunctors();
            void abortNotInLoopThread();

            std::atomic<bool> m_bLooping;
            bool m_bCallingPendingFactors;
            const std::thread::id m_threadId;    
            
            ChannelArray m_activeChannels;
            Channel* m_currentActiveChannel;

            std::time_t m_PollReturnTime;

            std::shared_ptr<Poller> m_poller;

            // pending factors
            mutable std::mutex m_pendingFactorLock;
            std::vector<Functor> m_pendingFunctors;

        };
    }
}
#endif