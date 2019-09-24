#ifndef _SEENET_SOCKET_OPS_H
#define _SEENET_SOCKET_OPS_H

#include "../seenet.h"
#include<arpa/inet.h>
// for ssize_t
#include<unistd.h>

namespace seenet{
    namespace net{
        namespace sockets{
            int createNonblockingOrDie(sa_family_t family);
            
            int connect(int sockFd, const struct sockaddr *addr);
            void bindOrDie(int sockFd, const struct sockaddr *addr);
            void listenOrDie(int sockFd);

            int accept(int sockFd, struct sockaddr_in6* addr);
            ssize_t read(int sockFd, void *buf, size_t cout);
            ssize_t readv(int sockFd, const struct iovec* iov, int iovcnt);
            ssize_t write(int sockFd, const void *buf, size_t count);
            void close(int sockFd);
            void shutdownWrite(int sockFd);
            void toIpPort(char *buf, size_t size,
                          const struct sockaddr* addr);
            void toIp(char *buf, size_t size, 
                      const struct sockaddr * addr);
            void fromIpPort(const char *ip, uint16_t port,
                            struct sockaddr_in *addr);
            void fromIpPort(const char *ip, uint16_t port,
                            struct sockaddr_in6* addr);

            int getSocketError(int sockFd);

            const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
            const struct sockaddr* sockeddr_cast(const struct sockaddr_in6* addr);
            struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
            const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr * addr);
            const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr *addr);

            struct sockaddr_in6 getLocalAddr(int sockFd);
            struct sockaddr_in6 getPeerAddr(int sockFd);

            bool isSelfConnect(int sockFd);
        };
    }
}

#endif