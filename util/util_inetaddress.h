#ifndef _SEENET_INETADDRESS_H
#define _SEENET_INETADDRESS_H

#include "../seenet.h"
#include "../noncopyable.h"
#include "../copyable.h"

#include <string>
#include <netinet/in.h>

namespace seenet{
    namespace net{
        namespace sockets{
            const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
        }

        class InetAddress: public Copyable
        {
        public:
            explicit InetAddress(uint16_t port = 0, bool loobackonly = false, bool ipv6 = false);
            InetAddress(std::string_view ip, uint16_t port, bool ipv6 = false);
            explicit InetAddress(const struct sockaddr_in& addr)
            :m_addr(addr)
            {

            }

            explicit InetAddress(const struct sockaddr_in6& addr)
            :m_addr6(addr)
            {

            }

            sa_family_t family() const { return m_addr.sin_family;}

            std::string toIp() const;
            std::string toIpPort() const;
            uint16_t toPort() const;

            const struct sockaddr* getSockAddr() const 
            {
                return sockets::sockaddr_cast(&m_addr6);
            }

            void setSockAddrInet6(const struct sockaddr_in6& addr6)
            {
                m_addr6 = addr6;
            }

            uint32_t ipNetEndian() const;
            uint16_t portNetEndian() const 
            {
                return m_addr.sin_port;
            }

            static bool resolve(std::string_view hostname, InetAddress* result);

            void setScopeId(uint32_t socpe_id);
        private:
            union 
            {
                struct sockaddr_in m_addr;
                struct sockaddr_in6 m_addr6;
            };
            
        };
    }
}
#endif