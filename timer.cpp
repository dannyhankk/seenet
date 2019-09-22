#include "timer.h"

namespace seenet{
    namespace net{
        // init
        std::atomic<int64_t> Timer::m_numCreated(0);

        void Timer::restart(std::time_t now)
        {
            // todo 
            
        }
    }
}