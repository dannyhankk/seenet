#ifndef _SEENET_TIMER_H
#define _SEENET_TIMER_H

#include "noncopyable.h"
#include "seenet.h"

#include <ctime>
#include <functional>
#include <atomic>

namespace seenet{
    namespace net{
        class Timer:public NonCopyable
        {
        public:
        using TimerCallback=std::function<void()>;
            Timer(const TimerCallback& cb, 
                  std::time_t when,
                  double interval)
                  : m_callback(cb),
                    m_expiration(when),
                    m_interval(interval),
                    m_bRepeat(interval > 0.0),
                    seq(++m_numCreated)
            {

            }
            ~Timer();
        private:
          const TimerCallback m_callback;
          std::time_t m_expiration;
          const double m_interval;
          const bool m_bRepeat;
          const int64_t seq;

          static std::atomic<int64_t> m_numCreated;


        };
    }
}
#endif