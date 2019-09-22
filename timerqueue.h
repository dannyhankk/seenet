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
        class TimerQueue:public NonCopyable
        {
        public:
        using TimerCallback=std::function<void()>;
            TimerQueue(EventLoop_sPt loop);
            ~TimerQueue();

            TimerId addTimer(const TimerCallback &cb,
                             std::time_t when,
                             double interval);
            void cancel(TimerId timerId);
        private:
            using TimerEntry=std::pair<std::time_t, Timer *>;
            using TimerList=std::set<TimerEntry>;
            
            using ActiveTimer=std::pair<Timer*, int64_t>;
            using ActiveTimerSet=std::set<ActiveTimer>;

            void addTimerInLoop(Timer* timer);
            void cancelInLoop(TimerId timerId);
            void handleRead();
            // move out all expired timers
            std::vector<TimerEntry> getExpired(std::time_t now);
            void reset(const std::vector<TimerEntry>& expired, std::time_t now);

            bool insert(Timer* timer);
            void cancelInLoop(TimerId timerId);;
            EventLoop_wPt m_wLoop;
            const int m_timerFd;
            Channel_wPt m_wTimerFdChannel;
            TimerList m_timers;

            //for cancel()
            ActiveTimerSet m_activeTimers;
            std::atomic<bool> m_callingExpiredTimers;
            ActiveTimerSet m_cancelingTimers;
        };
    }
}
#endif