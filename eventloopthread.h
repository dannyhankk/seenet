#ifndef _SEENET_EVENTLOOPTHREAD_H
#define _SEENET_EVENTLOOPTHREAD_H

#include "seenet.h"
#include "noncopyable.h"

#include <mutex>
#include <thread>
#include <condition_variable>

namespace seenet{
    namespace net{
        class EventLoop;

        class EventLoopThread: NonCopyable
        {
        public:
            using ThreadInitCallback = std::function<void(EventLoop_sPt&)>;

            EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                            const std::string& name = std::string());
            ~EventLoopThread();

            EventLoop_sPt startLoop();

        private:
            void threadFunc();
            

            bool m_bExisting;
            std::thread m_thread;

            std::mutex m_condLock;
            std::condition_variable m_condVar;
            EventLoop_sPt m_loop;

            ThreadInitCallback m_callback;    
            // thread name
            std::string m_threadName;
        };
    }
}

#endif