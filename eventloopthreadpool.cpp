#include "seenet.h"
#include "noncopyable.h"
#include "eventloopthread.h"
#include "eventloopthreadpool.h"
#include "eventloop.h"


#include <assert.h>

namespace seenet{
    namespace net{
        EventLoopThreadPool::EventLoopThreadPool(EventLoop_sPt baseLoop, const std::string& nameArg)
        : m_baseLoop(baseLoop), m_name(nameArg), m_started(false), m_numThreads(0), m_next(0)
        {

        }

        EventLoopThreadPool::~EventLoopThreadPool()
        {

        }

        void EventLoopThreadPool::start(const ThreadInitCallback& cb)
        {
            assert(m_started);
            m_baseLoop->assertInLoopThread();
            m_started = true;


            for (int i = 0 ; i < m_numThreads; i ++)
            {
                 unsigned int len = m_name.size() + 32;
                 std::shared_ptr<char> buf(new char[len]);
                 
                 snprintf(buf.get(), len, "%s%d", m_name.c_str(), i);
                 EventLoopThread *t = new EventLoopThread(cb, buf.get());
                 m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
                 m_loops.push_back(t->startLoop());
            }

            if(m_numThreads == 0 && cb)
            {
                cb(m_baseLoop);
            }
        }

        EventLoop_sPt EventLoopThreadPool::getNextLoop()
        {
            m_baseLoop->assertInLoopThread();
            assert(m_started);

            EventLoop_sPt loop = m_baseLoop;
            if(!m_loops.empty())
            {
                // round-robin
                loop = m_loops[m_next];
                ++m_next;

                if(static_cast<size_t>(m_next) >= m_loops.size())
                {
                    m_next = 0;
                }
            }

            return loop;
        }

        EventLoop_sPt EventLoopThreadPool::getLoopForHash(size_t hashCode)
        {
            m_baseLoop->assertInLoopThread();
            EventLoop_sPt loop = m_baseLoop;

            if(!m_loops.empty())
            {
                loop = m_loops[hashCode % m_loops.size()];
            }
            return loop;
        }

        std::vector<EventLoop_sPt> EventLoopThreadPool::getAllLoops()
        {
            m_baseLoop->assertInLoopThread();
            assert(m_started);
            if(m_loops.empty())
            {
                return std::vector<EventLoop_sPt>(1, m_baseLoop);
            }
            else 
            {
                return m_loops;
            }
        }
    }
}