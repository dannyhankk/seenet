#include "../seenet.h"
#include "./util_endian.h"
#include "util_socketops.h"
#include "util_inetaddress.h"

#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInadddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"
namespace seenet{
    namespace net{

        static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
                      "InetAddress is same size as sockaddr_in6");
        static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
        static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
        static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
        static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

        InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
        {
            static_assert(offsetof(InetAddress, m_addr6) == 0, "m_addr6 offset 0");
            static_assert(offsetof(InetAddress, m_addr) == 0, "m_addr offset 0");
            if(ipv6)
            {
                memset(&m_addr6, 0, sizeof(m_addr6));
                m_addr6.sin6_family = AF_INET6;
                in6_addr ip  = loopbackOnly ? in6addr_loopback: in6addr_any;
                m_addr6.sin6_addr = ip;
                m_addr6.sin6_port = sockets::hostToNetwork16(port);  
            }
            else
            {
                memset(&m_addr, 0, sizeof(m_addr));
                m_addr.sin_family = AF_INET;
                in_addr_t ip = loopbackOnly ? kInaddrLoopback: kInadddrAny;
                m_addr.sin_addr.s_addr = sockets::hostToNetwork32(ip);
                m_addr.sin_port = sockets::hostToNetwork16(port);
            }
        }

        InetAddress::InetAddress(std::string_view ip, uint16_t port, bool ipv6)
        {
            if(ipv6)
            {
                memset(&m_addr6, 0, sizeof(m_addr6));
                sockets::fromIpPort(ip.data(), port, &m_addr6);
            }
            else
            {
                memset(&m_addr, 0, sizeof(m_addr));
                sockets::fromIpPort(ip.data(), port, &m_addr);
            }
        }

        std::string InetAddress::toIpPort() const 
        {
            char buf[64] = "";
            sockets::toIpPort(buf, sizeof(buf), getSockAddr());
            return buf;
        }

        std::string InetAddress::toIp() const
        {
            char buf[64] = "";
            sockets::toIp(buf, sizeof(buf), getSockAddr());
            return buf;
        }

        uint32_t InetAddress::ipNetEndian() const
        {
            assert(family() == AF_INET);
            return m_addr.sin_addr.s_addr;
        }

        uint16_t InetAddress::toPort() const
        {
            return sockets::networkToHost16(portNetEndian());
        }

        static __thread  char t_resolveBuffer[64 * 1024];

        bool InetAddress::resolve(std::string_view hostname, InetAddress* out)
        {
            assert(out != NULL);
            struct hostent hent;
            struct hostent* he = NULL;
            int herrno = 0;
            memset(&hent, 0, sizeof(hent));

            int ret = gethostbyname_r(hostname.data(), &hent, t_resolveBuffer, 
                                      sizeof(t_resolveBuffer), &he, &herrno);
            if(ret == 0 && he != NULL)
            {
                assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
                out->m_addr.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
                return true;
            }
            else
            {
                if(ret)
                {
                    //to log
                }
                return false;
            }
        }

        void InetAddress::setScopeId(uint32_t socpe_id)
        {
            if(family() == AF_INET6)
            {
               m_addr6.sin6_scope_id = socpe_id; 
            }
        }
    }
}

