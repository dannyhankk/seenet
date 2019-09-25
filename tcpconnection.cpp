#include"seenet.h"
#include"noncopyable.h"
#include"tcpconnection.h"
#include "./util/util_inetaddress.h"
#include "./util/util_socket.h"
#include "channel.h"
#include "EventLoop.h"
#include "./util/util_socketops.h"

#include <memory>
#include <functional>

#include<unistd.h>

namespace seenet{
    namespace net{
        void defaultConnectionCallback(const TcpConnection_sPt conn)
        {
            //todo log address
        }

        void defaultMessageCallback(const TcpConnection_sPt& cb, Buffer* buf, std::time_t timestamp)
        {
            //buf->retriveall();
        }

        TcpConnection::TcpConnection(EventLoop_sPt loop,
                                     const std::string& nameArg,
                                     int sockFd, 
                                     const InetAddress& localAddr,
                                     const InetAddress& peeraddr)
        : m_loop(loop), m_name(nameArg), m_state(kConnecting),m_reading(true)
        , m_socket(new Socket(sockFd)), m_Channel(new Channel(loop,sockFd)), m_localAddr(localAddr)
        , m_peerAddr(peeraddr), m_highWaterMark(64*1024*1024)
        {
 
            m_Channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
            m_Channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
            m_Channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
            m_Channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));

            m_socket->setKeepAlive(true);
        }

        TcpConnection::~TcpConnection()
        {
            //log debug
            assert(m_state == kDisconnected);
        }

        bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const 
        {
             return m_socket->getTcpInfo(tcpi);
        }

        std::string TcpConnection::getTcpInfoString() const
        {
            char buf[1024];
            buf[0] = '\0';
            m_socket->getTcpInfoString(buf, sizeof(buf));
        }

        void TcpConnection::send(const void* data, int len)
        {
            send(std::string_view(static_cast<const char*>(data), len));
        }

        void TcpConnection::send(const std::string_view& message)
        {
            if(m_state == kConnected)
            {
                if(m_loop->isInLoopThread())
                {
                    sendInLoop(message);
                }
            }
            else
            {
                void(TcpConnection::*fp)(const std::string_view& message) = &TcpConnection::sendInLoop;
                m_loop->runInLoop(std::bind(fp, shared_from_this(), std::string(message.data(), message.size())));
            }
        }

        void TcpConnection::sendInLoop(const std::string_view& message)
        {
            sendInLoop(message.data(), message.size());
        }

        void TcpConnection::sendInLoop(const void* data, size_t len)
        {
            m_loop->assertInLoopThread();
            ssize_t nwrote = 0;
            size_t remaining = len;
            bool faultError = false;
            if(m_state == kDisconnected)
            {
                //log warn
                return;
            }

            if(!m_Channel->isWriting() && m_outputBuffer.readableBytes() == 0)
            {
                nwrote = sockets::write(m_Channel->fd(), data, len);  
                if(nwrote >= 0)
                {
                    remaining = len - nwrote;
                    if(remaining == 0 && m_writeCompleteCallback)
                    {
                        m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                    }
                }
                else 
                {
                    nwrote = 0;
                    if(errno != EWOULDBLOCK)
                    {
                        //log sys
                        if(errno != EPIPE || errno == ECONNRESET)
                        {
                            faultError = true;
                        }
                    }
                }
            }

            assert(remaining <= len);
            if(!faultError && remaining > 0)
            {
                size_t oldLen = m_outputBuffer.readableBytes();
                if(oldLen + remaining >= m_highWaterMark && m_highWateMarkCallback)
                {
                    m_loop->queueInLoop(std::bind(m_highWateMarkCallback, shared_from_this(), oldLen + remaining));
                }
                outputBuffer.append();

                if(m_Channel->isWriting())
                {
                    m_Channel->enableWriting();
                }

            }

        }

    }
}