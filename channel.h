#ifndef _SEENET_CHANNEL_H
#define _SEENET_CHANNEL_H
#include "seenet.h"
#include <functional>
#include "noncopyable.h"


namespace seenet{
    namespace net{
		class EventLoop;

        class Channel:public NonCopyable,
                      public std::enable_shared_from_this<Channel>
        {
        public:
            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;
            using EventCallback=std::function<void()>;
            using ReadEventCallback=std::function<void(std::time_t)>;

            Channel(EventLoop* loop, int fd);

            void handleEvent();

            void setReadCallback(const ReadEventCallback& cb){ m_readCallback = cb; }

            void setWriteCallback(const EventCallback& cb){ m_writeCallback = cb; }

            void setErrorCallback(const EventCallback& cb){ m_errorCallback = cb; }

            void setCloseCallback(const EventCallback& cb){ m_closeCallback = cb;}

            void enableReading(){ m_events |= kReadEvent; update();}
            void disableReading(){ m_events &= ~kReadEvent; update();}

            void enableWriting(){ m_events |= kWriteEvent; update();}
            void disableWriting(){ m_events &= ~ kWriteEvent; update();}
            
            void disableAll(){ m_events = kNoneEvent; update();}

            bool isWriting() const { return m_events & kWriteEvent;}
            bool isReading() const { return m_events & kReadEvent;}
            bool isNonEvent() const{ return m_events == kNoneEvent;}

            int fd() const { return m_fd;}

            int events() const{return m_events;}

            void set_revents(int revt){ m_revents = revt;}
            
            //
            EventLoop* ownerLoop(){ return m_loop;}
            void remove();
            // for Poller
            int index() {return m_index;}
            void set_idnex(int idx){m_index = idx;}
        private:
            void update();

            // callbacks
            ReadEventCallback m_readCallback;
            EventCallback m_writeCallback;
            EventCallback m_errorCallback;
            EventCallback m_closeCallback;

            //for poller
            const int m_fd;
            EventLoop* m_loop;
            
            int m_events;
            int m_revents;
            int m_index;
        };

    }
}
#endif