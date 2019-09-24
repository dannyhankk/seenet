#ifndef _SEENET_SOCKET_H
#define _SEENET_SOCKET_H

#include "../seenet.h"
#include "../noncopyable.h"


namespace seenet{

    namespace net {
        class InetAddress;

        class Socket: public NonCopyable
        {
        public:
            explicit Socket(int sockFd): m_socketFd(sockFd)
            {

            }
            ~Socket();

            int fd() const { return m_socketFd;}
            bool getTcpInfo( struct tcp_info *) const;
            bool getTcpInfoString(char *buf, int len) const;
            void bindAddress(const InetAddress& localAddr);
            void listen();
            int accept(InetAddress *peerAddr);
            void shutdownWrite();

            void setTcpNoDelay(bool on);
            void setReuseAddr(bool on);
            void setReusePort(bool on);
            void setKeepAlive(bool on);
        private:
            const int m_socketFd;
        };
    }
}


#endif