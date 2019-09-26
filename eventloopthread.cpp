#include"seenet.h"
#include"noncopyable.h"
#include"eventloop.h"
#include "eventloopthread.h"
#include <assert.h>

namespace seenet{
    namespace net{
        EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name):
        // for now ignore thread name
        m_loop(NULL), m_bExisting(false), m_thread(nullptr),
          m_callback(cb), m_threadName(name)
          {

          }

        EventLoopThread::~EventLoopThread()
        {
            m_bExisting = true;
            if(m_loop != NULL && m_loop.get() != NULL)
            {
                m_loop->quit();
                m_thread.join();
            }
        }

        EventLoop_sPt EventLoopThread::startLoop()
        {
           assert(!m_thread.joinable());
           // start thread, and wait for loop instance created
           m_thread = std::move(std::thread(&EventLoopThread::threadFunc, this));
           {
               std::unique_lock<std::mutex> lock(m_condLock);
               m_condVar.wait(lock,[&]{ return m_loop != NULL;});
           }

           return m_loop;

        }

        void EventLoopThread::threadFunc()
        {
            EventLoop_sPt loop = std::make_shared<EventLoop>(new EventLoop());
            if(m_callback)
            {
                m_callback(loop);
            }
            {
                std::unique_lock<std::mutex> lock(m_condLock);
                m_loop = loop;
                m_condVar.notify_one();
            }

            //start m_loop
            loop->loop();
            m_loop.reset();
        }
    }
}
