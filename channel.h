#ifndef _SEENET_CHANNEL_H
#define _SEENET_CHANNEL_H
#include "seenet.h"
#include <functional>
#include "noncopyable.h"
class EventLoop;

namespace seenet{
    namespace net{
        class Channel:public NonCopyable,
        std::enable_shared_from_this<Channel>
        {
        public:
            using EventCallback=std::function<void()>;

            Channel(EventLoop_sPt loop, int fd);

            void handleEvent();

            void setReadCallback(const EventCallback& cb){ m_readCallback = cb; }

            void setWriteCallback(const EventCallback& cb){ m_writeCallback = cb; }

            void setErrorCallback(const EventCallback& cb){ m_errorCallback = cb; }

            void enableReading(){ m_events |= kReadEvent; update();}
            void disableReading(){ m_events &= ~kReadEvent; update();}

            void enableWriting(){ m_events |= kWriteEvent; update();}
            void disableWriting(){ m_events &= ~ kWriteEvent; update();}
            
            void disableAll(){ m_events = kNoneEvent; update();}

            bool isWriting() const { return m_events & kWriteEvent;}
            bool isReading() const { return m_events & kReadEvent;}
             bool isNoneEvent() const{ return m_events == kNoneEvent;}

            int fd() const { return m_fd;}

            int events() const{return m_events;}

            void set_revents(int revt){ m_events = revt;}
            
            //
            EventLoop_sPt ownerLoop(){ return m_loop.lock();}
            // for Poller
            int index() {return m_index;}
            void set_idnex(int idx){m_index = idx;}
        private:
            void update();

            // callbacks
            EventCallback m_readCallback;
            EventCallback m_writeCallback;
            EventCallback m_errorCallback;

            //for poller
            const int m_fd;
            EventLoop_wPt m_loop;
            
            int m_events;
            int m_revents;
            int m_index;

            //
            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;
        };

    }
}
#endif