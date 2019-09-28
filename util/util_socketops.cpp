#include "../seenet.h"
#include "../noncopyable.h"
#include "util_socketops.h"
#include "util_types.h"
#include "util_endian.h"


#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>


namespace seenet{
    namespace net{
        namespace sockets{
            using SA = struct sockaddr;

            #if VALGRIND || defined (NO_ACCEPT4)
            void setNonBlockAndCloseOnExec(int sockfd)
            {
                //non-block
                int flags = ::fcntl(sockFd, F_GETFL, 0);
                flags |= O_NONBLOCK;
                int ret = ::fcntl(sockFd, F_SETFL, flags);

                flags = ::fcntl(sockFd, F_GETFD, 0);
                flags |= FD_CLOEXEC;
                ret = ::fcntl(sockFd, F_SETFD, flags);
                (void)ret;
            }
            #endif

            const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr)
            {
                return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
            }

            struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr)
            {
                return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
            }

            const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
            {
                return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
            }

            const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr)
            {
                return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
            }

            const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr * addr)
            {
                return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
            }

            int createNonblockingOrDie(sa_family_t family)
            {
            #if VALGRIND
                int sockFd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
                if(sockFd < 0)
                {
                    //log sys
                }

                setNonBlockAndCloseOnExec(sockFd);
            #else
                int sockFd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
                if(sockFd < 0)
                {
                    //log sys
                }
            #endif
                return sockFd;
            }

            void bindOrDie(int sockFd, const struct sockaddr *addr)
            {
                int ret = ::bind(sockFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
                if(ret < 0)
                {
                    //log sys
                }
            }

            void listenOrDie(int sockFd)
            {
               int ret = ::listen(sockFd, SOMAXCONN);
               if(ret < 0)
               {
                   //log sys
               } 
            }

            int accept(int sockFd, struct sockaddr_in6* addr)
            {
                socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
                #if VALGRIND || defined(NO_ACCEPT4)
                  int connfd = ::accept(sockFd, sockaddr_cast(addr) &addrlen);
                  setNonBlockAndCloseOnExec(connfd);
                #else
                  int connfd = ::accept4(sockFd, sockaddr_cast(addr),
                                         &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
                #endif
                if(connfd < 0)
                {
                    int savedErrno = errno;
                    //log sys
                    switch(savedErrno)
                    {
                        case EAGAIN:
                        case ECONNABORTED:
                        case EINTR:
                        case EPROTO:
                        case EPERM:
                        case EMFILE:
                            errno = savedErrno;
                            break;
                        case EBADF:
                        case EFAULT:
                        case EINVAL:
                        case ENFILE:
                        case ENOBUFS:
                        case ENOMEM:
                        case ENOTSOCK:
                        case EOPNOTSUPP:
                        //log fatal
                            break;
                        default:
                            break;
                    }
                }

                return connfd;
            }

            int connect(int sockFd, const struct sockaddr *addr)
            {
                return ::connect(sockFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
            }

            ssize_t read(int sockFd, void* buf, size_t count)
            {
                return ::read(sockFd, buf, count);
            }

            ssize_t readv(int sockFd, const struct iovec* iov, int iovcnt)
            {
                return ::readv(sockFd, iov, iovcnt);
            }

            ssize_t write(int sockFd, const void *buf, size_t count)
            {
                return ::write(sockFd, buf, count);
            }

            void close(int sockFd)
            {
                if(::close(sockFd) < 0)
                {
                    //log sys
                }
            }

            void shutdownWrite(int sockFd)
            {
                if(::shutdown(sockFd, SHUT_WR) < 0)
                {
                    // log sys
                }
            }

            void toIpPort(char *buf, size_t size, const struct sockaddr* addr)
            {
                toIp(buf, size, addr);
                size_t end = ::strlen(buf);
                const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
                uint16_t port = networkToHost16(addr4->sin_port);
                assert(size > end);
                snprintf(buf+end, size-end, ":%u", port);
            }

            void toIp(char *buf, size_t size, const struct sockaddr *addr)
            {
                if(addr->sa_family == AF_INET)
                {
                    assert(size >= INET_ADDRSTRLEN);
                    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
                    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
                }
                else if(addr->sa_family == AF_INET6)
                {
                    assert(size > INET6_ADDRSTRLEN);
                    const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
                    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
                }
            }

            void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
            {
                addr->sin_family = AF_INET;
                addr->sin_port = hostToNetwork16(port);
                if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
                {
                    //log sys
                }
            }

            void fromIpPort(const char *ip, uint16_t port, struct sockaddr_in6* addr)
            {
                addr->sin6_family == AF_INET6;
                addr->sin6_port = hostToNetwork16(port);
                if(::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
                {
                    //log sys
                }
            }

            int getSocketError(int sockFd)
            {
                int optval;
                socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
                if(::getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
                {
                    return errno;
                }
                else
                {
                    return optval;
                }
            }

            struct sockaddr_in6 getLocalAddr(int sockFd)
            {
                struct sockaddr_in6 localAddr;
                memset(&localAddr, 0, sizeof(localAddr));
                socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
                if(::getsockname(sockFd, sockaddr_cast(&localAddr), &addrlen))
                {
                    //log sys
                }

                return localAddr;
            }

            struct sockaddr_in6 getPeerAddr(int sockFd)
            {
                struct sockaddr_in6 peeraddr;
                memset(&peeraddr, 0, sizeof(peeraddr));
                socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));
                if(::getpeername(sockFd, sockaddr_cast(&peeraddr), &addrlen) < 0)
                {
                    //log sys
                }
                return peeraddr;
            }

            bool isSelfConnect(int sockFd)
            {
                struct sockaddr_in6 localAddr = getLocalAddr(sockFd);
                struct sockaddr_in6 peeraddr = getLocalAddr(sockFd);
                if(localAddr.sin6_family == AF_INET)
                {
                    const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localAddr);
                    const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
                    return laddr4->sin_port == raddr4->sin_port &&
                        laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
                }
                else if(localAddr.sin6_family == AF_INET6)
                {
                    return localAddr.sin6_port == peeraddr.sin6_port && 
                            memcmp(&localAddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localAddr.sin6_addr));
                }
                else
                {
                    return false;
                }
            }
        }
        
    }
}


