#ifndef _SEENET_TIMERQUEUE_H
#define _SEENET_TIMERQUEUE_H

#include <functional>
#include <ctime>
#include <set>
#include <atomic>

#include "noncopyable.h"
#include "seenet.h"
#include "timerid.h"

namespace seenet{
    namespace net{

        class EventLoop;
        class Timer;
        class TimerId;

        class TimerQueue:public NonCopyable
        {
        public:
            using TimerEntry=std::pair<std::time_t, Timer_sPt>;
            using TimerSet=std::set<TimerEntry>;
            
            using ActiveTimer=std::pair<Timer_sPt, int64_t>;
            using ActiveTimerSet=std::set<ActiveTimer>;
           explicit TimerQueue(EventLoop_sPt loop);
            ~TimerQueue();

            TimerId addTimer(const TimerCallback cb,
                             std::time_t when,
                             double interval);
            void cancel(TimerId timerId);
        private:


            void addTimerInLoop(Timer_sPt timer);
            void cancelInLoop(TimerId timerId);
            void handleRead();
            // move out all expired timers
            std::vector<TimerEntry> getExpired(std::time_t now);
            void reset(const std::vector<TimerEntry>& expired, std::time_t now);

            bool insert(Timer_sPt timer);
            EventLoop_wPt m_wLoop;
            const int m_timerFd;
            Channel_sPt m_sTimerChannel;
            TimerSet m_timers;

            //for cancel()
            ActiveTimerSet m_activeTimers;
            std::atomic<bool> m_callingExpiredTimers;
            ActiveTimerSet m_cancelingTimers;
        };
    }
}
#endif