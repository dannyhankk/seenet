#include "../seenet.h"
#include "util_socket.h"
#include "util_socketops.h"
#include "util_inetaddress.h"

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

namespace seenet{
    namespace net{
        Socket::~Socket()
        {
            sockets::close(m_socketFd);
        }

        bool Socket::getTcpInfo(struct tcp_info* tcpi) const
        {
          socklen_t len = sizeof(*tcpi);
          memset((void *)tcpi, 0, (size_t)len);

          return ::getsockopt(m_socketFd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
        }

        bool Socket::getTcpInfoString(char *buf, int len) const 
        {
            struct tcp_info tcpi;
            bool ok = getTcpInfo(&tcpi);
            if(ok)
            {
                snprintf(buf, len, "unrecovered=%u "
                    "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                    "lost=%u retrans=%u rtt=%u rttvar=%u "
                    "sshthresh=%u cwnd=%u total_retrans=%u",
                    tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                    tcpi.tcpi_rto,          // Retransmit timeout in usec
                    tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
                    tcpi.tcpi_snd_mss,
                    tcpi.tcpi_rcv_mss,
                    tcpi.tcpi_lost,         // Lost packets
                    tcpi.tcpi_retrans,      // Retransmitted packets out
                    tcpi.tcpi_rtt,          // Smoothed round trip time in usec
                    tcpi.tcpi_rttvar,       // Medium deviation
                    tcpi.tcpi_snd_ssthresh,
                    tcpi.tcpi_snd_cwnd,
                    tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
            }
            return ok;
        }

        void Socket::bindAddress(const InetAddress& addr)
        {
            sockets::bindOrDie(m_socketFd, addr.getSockAddr());
        }

        void Socket::listen()
        {
            sockets::listenOrDie(m_socketFd);
        }

        int Socket::accept(InetAddress* peeraddr)
        {
            struct sockaddr_in6 addr;
            memset(&addr,0, sizeof(addr));

            int connFd = sockets::accept(m_socketFd, &addr);
            if (connFd >= 0)
            {
                peeraddr->setSockAddrInet6(addr);
            }

            return connFd;
        }

        void Socket::shutdownWrite()
        {
            sockets::shutdownWrite(m_socketFd);
        }

        void Socket::setTcpNoDelay(bool on)
        {
            int optval = on ? 1: 0;
            ::setsockopt(m_socketFd, IPPROTO_TCP, TCP_NODELAY, 
                           &optval, static_cast<socklen_t>(sizeof(optval)));
        }

        void Socket::setReuseAddr(bool on)
        {
            int optval = on ? 1: 0;
            ::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR,
                        &optval, static_cast<socklen_t>(sizeof(optval)));
        }

        void Socket::setReusePort(bool on)
        {
            int optval = on ? 1 : 0;
            int ret = ::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEPORT,
                                   &optval, static_cast<socklen_t>(sizeof(optval)));
            if(ret < 0 && on)
            {
                //todo log
            }
            
        }

        void Socket::setKeepAlive(bool on)
        {
            int optval = on ? 1: 0;
            ::setsockopt(m_socketFd, SOL_SOCKET, SO_KEEPALIVE,
                        &optval, static_cast<socklen_t>(sizeof(optval)));
        }
    }
}