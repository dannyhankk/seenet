#ifndef _SEENET_TCPCONNECTION_H
#define _SEENET_TCPCONNECTION_H

#include "seenet.h"
#include "noncopyable.h"
#include "util/util_inetaddress.h"
#include <memory>
#include <string>
#include <ctime>

namespace seenet{
    namespace net{
        class Channel;
        class EventLoop;
        class Socket;


        class TcpConnection:public NonCopyable, std::enable_shared_from_this<TcpConnection>
        {
        public:
            TcpConnection(EventLoop_sPt loop, const std::string& name, int sockFd,
                          const InetAddress& localAddr, const InetAddress& peeraddr);
            ~TcpConnection();

            EventLoop_sPt getLoop() const{return m_loop;}
            const std::string& name() const {return m_name;}
            const InetAddress& localAddress() const {return m_localAddr;}
            const InetAddress& peerAddress() const {return m_peerAddr;}

            bool connected() const { return m_state == kConnected;}
            bool Disconnected() const{ return m_state == kDisconnected;}

            // return true if success
            bool getTcpInfo(struct tcp_ifo*) const;
            std::string getTcpInfoString() const;
            //void
            void send(const void* message, int len);


        private:
            enum StateE{kDisconnected, kConnecting, kConnected, kDisconnecting};
            void handleRead(std::time_t now);
            void handleWrite();
            void handleClose();
            void handleError();

            //void sendInLoop(string&& message)
            void sendInLoop(const std::string_view& message);
            void sendInLoop(const void *message, size_t len);
            void shutdownInLoop();

            //void
            void forceCloseInLoop();
            void setState(StateE s) { m_state = s;}
            const char* stateToString() const;
            void startReadInLoop();
            void stopReadInLoop();

            EventLoop_sPt m_loop;
            const std::string m_name;
            StateE m_state;
            bool m_reading;

            //for
            std::unique_ptr<Socket> m_socket;
            std::unique_ptr<Channel> m_Channel;
            const InetAddress m_localAddr;
            const InetAddress m_peerAddr; 
        };
    }
}
#endif