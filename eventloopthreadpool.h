#ifndef _SEENET_EVENTLOOPTHREADPOOL_H
#define _SEENET_EVENTLOOPTHREADPOOL_H


#include "seenet.h"
#include "noncopyable.h"

#include <functional>
#include <memory>
#include <vector>

namespace seenet{
    namespace net{
        class EventLoop;
        class EventLoopThread;

        class EventLoopThreadPool: NonCopyable
        {
         public:
             EventLoopThreadPool(EventLoop_sPt loop, const std::string& nameArg);
             ~EventLoopThreadPool();

             void setThreadNum(int numThreads) { m_numThreads = numThreads;}
             void start(const ThreadInitCallback& cb = ThreadInitCallback());

             EventLoop_sPt getNextLoop();
             EventLoop_sPt getLoopForHash(size_t hashCode);

             std::vector<EventLoop_sPt> getAllLoops();

             bool started() const
             {
                 return m_started;
             }

             const std::string& name() const
             {
                 return m_name;
             }

         private:
             EventLoop_sPt m_baseLoop;
             std::string m_name;
             bool m_started;
             int m_numThreads;
             int m_next;
             std::vector<std::unique_ptr<EventLoopThread>> m_threads;
             std::vector<EventLoop_sPt> m_loops;
        };
    }
}
#endif