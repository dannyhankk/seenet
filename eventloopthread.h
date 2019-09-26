#ifndef _SEENET_EVENTLOOPTHREAD_H
#define _SEENET_EVENTLOOPTHREAD_H

#include "seenet.h"
#include "noncopyable.h"

#include <mutex>

namespace seenet{
    namespace net{
        class EventLoop;

        class EventLoopThread: NonCopyable
        {
        public:
            using ThreadInitCallback = std::function<void(EventLoop_sPt)>;

            EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                            const std::string& name = std::string());
            ~EventLoopThread();

            EventLoop_sPt startLoop();

        private:
            void threadFunc();

            std::mutex m_loopLock;
            EventLoop_sPt m_loop;

            bool m_bExisting;
            std::thread::id m_threadID;

            std::mutex m_condLock;
            std::condition_variable m_cond;

            ThreadInitCallback m_callback;    
        };
    }
}

#endif