#ifndef _SENET_TYPES_H
#define _SENET_TYPES_H

#include<stdint.h>
#include<string.h>
#include<string>

namespace seenet{

    template<typename To, typename From>
    inline To implicit_cast(From const &f)
    {
        return f;
    }

    template<typename To, typename From>
    inline To down_cast(From* f)
    {
        if(false)
        {
            implicit_cast<From*, To>(0);
        }

        return static_cast<To>(f);
    }
}
#endif