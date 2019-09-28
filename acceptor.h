#ifndef _SEENET_ACCEPTOR_H
#define _SEENET_ACCEPTOR_H

#include "seenet.h"
#include "noncopyable.h"
#include "util/util_socket.h"

#include <functional>


namespace seenet{
    namespace net{
        class EventLoop;
        class InetAddress;

        class Acceptor: public std::enable_shared_from_this<Acceptor>, public NonCopyable
        {
        public:
            using NewConnectionCallback = std::function<void(int sockFd, const InetAddress&)>;
            Acceptor(EventLoop_sPt loop, const InetAddress& listenAddr, bool reuseport);
            ~Acceptor();

            void setNewConnectionCallback(const NewConnectionCallback& cb)
            {
                m_newConnectionCallback = cb;
            }

            bool listening() const {return m_bListening;}
            void listen();
        private:
            void handleRead();

            EventLoop_sPt m_loop;
            Socket m_acceptSocket;
            Channel_sPt m_acceptChannel;
            NewConnectionCallback m_newConnectionCallback;
            bool m_bListening;
            int m_idleFd;

        };
    }
}
#endif