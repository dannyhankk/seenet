#include "timerqueue.h"

#include "channel.h"
#include "eventloop.h"
#include "timer.h"
#include "timerid.h"

#include <ctime>
#include <unistd.h>
#include <sys/timerfd.h>
#include <assert.h>

namespace seenet{
    namespace net{

	   namespace detail{

	   int createTimerfd()
	   {
		  int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
		  if(timerFd < 0)
		  { 
			  //todo log error
		  }

		  return timerFd;
	   }
 
	   }

	   
        TimerQueue::TimerQueue(EventLoop_sPt loop)
        : m_wLoop(loop),
          m_timerFd(detail::createTimerfd()),
          m_sTimerChannel(new Channel(loop, m_timerFd)),
          m_timers(),
          m_callingExpiredTimers(false)
        {
           m_sTimerChannel->setReadCallback(std::bind(&TimerQueue::handleRead, this));
		   m_sTimerChannel->enableReading();
        }

		TimerQueue::~TimerQueue()
        {
           m_sTimerChannel->disableAll();
		   m_sTimerChannel->remove();
		   ::close(m_timerFd);
		   for (const TimerEntry& timer: m_timers)
		   {
              delete timer.second.get();
		   }
		}

		TimerId TimerQueue::addTimer(const TimerCallback cb, std::time_t when, double interval)
		{
           
           Timer_sPt timer = std::make_shared<Timer>(std::move(cb), when, interval);
		   auto loop = m_wLoop.lock();
		   if(loop)
		   {
		   	 loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
			 return TimerId(timer, timer->sequence());
		   }
		   
		}

		void TimerQueue::addTimerInLoop(Timer_sPt timer)
		{
            auto loop = m_wLoop.lock();
			if(loop)
			{
				loop->assertInLoopThread();
				bool earliestChanged = insert(timer);

				if(earliestChanged)
				{
					//resetTimerfd(m_timerFd, timer->expiration());
				}
			}
		}

		void TimerQueue::cancelInLoop(TimerId timerId)
		{
			auto loop = m_wLoop.lock();
			if(loop)
			{
				loop->assertInLoopThread();
				assert(m_timers.size() == m_activeTimers.size());

				ActiveTimer timer(timerId.m_timer, timerId.m_seq);

				ActiveTimerSet::iterator it = m_activeTimers.find(timer);
				if(it != m_activeTimers.end())
				{
					size_t n = m_timers.erase(TimerEntry(it->first->expiration(), it->first));
					assert(n == 1);
					delete it->first.get();
					m_activeTimers.erase(it);
				}
				else if(m_callingExpiredTimers)
				{
					m_cancelingTimers.insert(timer);
				}

				assert(m_timers.size() == m_activeTimers.size());
			}
		}

		void TimerQueue::handleRead()
		{
			auto loop = m_wLoop.lock();
			if(loop)
			{
			   std::time_t now(std::time(nullptr));
			   //readTimerfd()
			}

		}

		std::vector<TimerQueue::TimerEntry> TimerQueue::getExpired(std::time_t now)
		{
			assert(m_timers.size() == m_activeTimers.size());
			std::vector<TimerEntry> expired;
			
		}



		
    } // namespace net 
}
