#ifndef _SEENET_COPYABLE_H
#define _SEENET_COPYABLE_H

namespace seenet{
    namespace net{
        class Copyable
        {
         protected:
             Copyable() = default;
             ~Copyable() = default;
        };
    }
}
#endif