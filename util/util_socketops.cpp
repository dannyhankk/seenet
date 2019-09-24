#include "../seenet.h"
#include "../noncopyable.h"
#include "util_socketops.h"


#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

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
            return static_cast<const struct sockaddr*>(static_cast<const void*>(addr));
        }

        struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr)
        {
            return static_cast<struct sockaddr*>(static_cast<void*>(addr));
        }

        }   
        
    }
}


