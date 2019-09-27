#include "seenet.h"
#include "./util/util_socketops.h"
#include "buffer.h"

namespace seenet{
    namespace net{


        const char Buffer::m_kCRLF[] = "\r\n";
        const size_t Buffer::kCheapPrepend;
        const size_t Buffer::kInitialSize;

        ssize_t Buffer::readFd(int fd, int *saveErrno)
        {
            char extraBuf[65536];
            struct iovec vec[2];

            const size_t writable = wirtableBytes();
            vec[0].iov_base = begin() + m_wirterIndex;
            vec[0].iov_len = writable;

            vec[1].iov_base = extraBuf;
            vec[1].iov_len = sizeof(extraBuf);

            const int iovcnt = (writable < sizeof(extraBuf))?2:1;
            const ssize_t n = sockets::readv(fd, vec, iovcnt);
            if(n < 0)
            {
                *saveErrno = errno;
            }
            else if(static_cast<size_t>(n) <= writable)
            {
                m_wirterIndex += n;
            }
            else
            {
                m_wirterIndex = m_buff.size(); 
                append(extraBuf, n - writable);
            }
        }

    }
}