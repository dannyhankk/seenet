#ifndef _SEENET_TIMERID_H
#define _SEENET_TIMERID_H

#include "noncopyable.h"
#include "seenet.h"

#include <memory>

namespace seenet{
    namespace net{
        class Timer;

        class TimerId
        {
        public:
            TimerId()
            : m_seq(0)
              {

              }
            TimerId(Timer_sPt timer, int64_t seq)
            : m_timer(timer), m_seq(seq)
            {

            }

            friend class TimerQueue;
        private:
            Timer_wPt m_timer;
            int64_t m_seq;
        };
    }
}

#endif