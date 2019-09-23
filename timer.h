#ifndef _SEENET_TIMER_H
#define _SEENET_TIMER_H

#include "seenet.h"
#include "noncopyable.h"

#include <ctime>
#include <functional>
#include <atomic>

namespace seenet{
    namespace net{
        class Timer:public NonCopyable
        {
        public:
            Timer(const TimerCallback& cb, std::time_t when, double interval)
            :m_callback(cb), m_expiration(when),
                    m_interval(interval),
                    m_bRepeat(interval > 0.0),
                    m_seq(++m_numCreated)
            {

            }

            void run() const
            {
              m_callback();
            }

            std::time_t expiration() const 
            {
              return m_expiration;
            }

            bool repeat() const 
            {
              return m_bRepeat;
            }

            int64_t sequence() const {return m_seq;}

            void restart(std::time_t now);

            static int64_t numCreated()
            {
              return m_numCreated.load();
            }
            
        private:
          const TimerCallback m_callback;
          std::time_t m_expiration;
          const double m_interval;
          const bool m_bRepeat;
          const int64_t m_seq;
          static std::atomic<int64_t> m_numCreated;
        };
    }
}
#endif