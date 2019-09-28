#ifndef _SEENET_TCPCONNECTION_H
#define _SEENET_TCPCONNECTION_H

#include "seenet.h"
#include "noncopyable.h"
#include "util/util_inetaddress.h"
#include "buffer.h"
#include <memory>
#include <string>
#include <ctime>
#include <any>
#include <netinet/tcp.h>

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
            bool getTcpInfo(struct tcp_info*) const;
            std::string getTcpInfoString() const;
            //void
            void send(const void* message, int len);
            void send(const std::string_view& message);

            //void
            // wait for buff
            void send(Buffer* message); 
            void shutdown();
            void forceClose();
            void forceCloseWithDelay(double seconds);
            void setTcpNoDelay(bool on);
            // reading or not
            void startRead();
            void stopRead();
            bool isReading() const { return m_reading;}

            void setContext(const std::any& context)
            {
                m_context = context;
            }
            const std::any* getContext() const 
            {
                return &m_context;
            }
            
            std::any* getMutableContext()
            {
                return &m_context;
            }

            void setNewConnectionCallback(const ConnectionCallback& cb)
            {
                m_connCallback = cb;
            }

            void setMessageCallback(const MessageCallback& cb)
            {
                m_messageCallback = cb;
            }

            void setWriteCompleteCallback(const WriteCompleteCallback& cb)
            {
                m_writeCompleteCallback = cb;
            }

            void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
            {
                m_highWateMarkCallback = cb;
                m_highWaterMark = highWaterMark;
            }

            Buffer* inputBuffer()
            {
                return &m_inputBuffer;
            }

            Buffer* outputBuffer()
            {
                return &m_outputBuffer;
            }

            void setCloseCallback(const CloseCallback& cb)
            {
                m_closeCallback = cb;
            }

            void connectEstablished();
            void connectDestroyed();

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

            ConnectionCallback m_connCallback;
            MessageCallback m_messageCallback;
            WriteCompleteCallback m_writeCompleteCallback;
            HighWaterMarkCallback m_highWateMarkCallback;
            CloseCallback m_closeCallback;

            size_t m_highWaterMark;
            Buffer m_inputBuffer;
            Buffer m_outputBuffer;
            std::any m_context;


        };
    }
}
#endif