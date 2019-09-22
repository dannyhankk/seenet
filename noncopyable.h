#ifndef _SEENET_NONCOPYABLE_H
#define _SEENT_NONCOPYABLE_H


namespace seenet{
    namespace net {
        class NonCopyable
        {
        protected:
            NonCopyable(){}
            ~NonCopyable(){}
        private:
           NonCopyable(const NonCopyable&);
           NonCopyable& operator=(const NonCopyable&);
        };
    }
}
#endif