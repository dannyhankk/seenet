#include "seenet.h"
#include "timerqueue.h"

#include "channel.h"
#include "eventloop.h"
#include "timer.h"
#include "timerid.h"

#include <ctime>
#include <unistd.h>
#include <sys/timerfd.h>
// for CLOCK_* macro define
//#include <linux/time.h>
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

        void TimerQueue::reset(const std::vector<TimerEntry>& expired, std::time_t now)
		{
			std::time_t nextExpire;
			for(std::vector<TimerEntry> ::const_iterator it = expired.begin();
			   it != expired.end(); ++it)
			   {
				   ActiveTimer timer(it->second, it->second->sequence());
				   if(it->second->repeat() &&
				      m_cancelingTimers.find(timer) == m_cancelingTimers.end())
					  {
						  it->second->restart(now);
						  insert(it->second);
					  }
					  else
					  {
						  delete it->second.get();
					  }
					  
			   }

			if(!m_timers.empty())
			{
				nextExpire = m_timers.begin()->second->expiration();
			}

			if(nextExpire > 0)
			{
                //reset timerfd
			}
		}

		bool TimerQueue::insert(Timer_sPt timer)
		{
			auto loop = m_wLoop.lock();
			if(loop)
			{
				loop->assertInLoopThread();
				assert(m_timers.size() == m_activeTimers.size());
				bool earliestChanged = false;
				std::time_t when = timer->expiration();
				TimerSet::iterator it = m_timers.begin();
				if(it == m_timers.end() || when < it->first)
				{
					earliestChanged = true;
				}
				{
					std::pair<TimerSet::iterator, bool> result = m_timers.insert(TimerEntry(when, timer));
					assert(result.second);
				}
				{
					std::pair<ActiveTimerSet::iterator, bool> result 
					    = m_activeTimers.insert(ActiveTimer(timer, timer->sequence()));
						assert(result.second); 
				}
				assert(m_timers.size() == m_activeTimers.size());
				return earliestChanged;
			}
		}	
    } // namespace net 
}
