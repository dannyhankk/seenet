#include"seenet.h"
#include"noncopyable.h"
#include"tcpconnection.h"
#include "./util/util_inetaddress.h"
#include "./util/util_socket.h"
#include "channel.h"
#include "EventLoop.h"
#include "./util/util_socketops.h"
#include "buffer.h"

#include <memory>
#include <functional>

#include<unistd.h>

namespace seenet{
    namespace net{
        void defaultConnectionCallback(const TcpConnection_sPt& conn)
        {
            //todo log address
        }

        void defaultMessageCallback(const TcpConnection_sPt& conn, Buffer* buf, std::time_t receiveTime)
        {
            //buf->retriveall();
        }

        TcpConnection::TcpConnection(EventLoop_sPt loop,
                                     const std::string& nameArg,
                                     int sockFd, 
                                     const InetAddress& localAddr,
                                     const InetAddress& peeraddr)
        : m_loop(loop), m_name(nameArg), m_state(kConnecting),m_reading(true)
        , m_socket(new Socket(sockFd)), m_Channel(std::make_shared<Channel>(loop.get(),sockFd)), m_localAddr(localAddr)
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
                else
                {
                    void(TcpConnection::*fp)(const std::string_view& message) = &TcpConnection::sendInLoop;
                    m_loop->runInLoop(std::bind(fp, shared_from_this(), std::string(message.data(), message.size())));
                }
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
                m_outputBuffer.append(static_cast<const char*>(data)+nwrote, remaining);

                if(m_Channel->isWriting())
                {
                    m_Channel->enableWriting();
                }
            }

        }

        void TcpConnection::shutdown()
        {
            if(m_state == kConnected)
            {
                setState(kDisconnecting);
                m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
            }
        }

        void TcpConnection::shutdownInLoop()
        {
            m_loop->assertInLoopThread();
            if(m_Channel->isWriting())
            {
                m_socket->shutdownWrite();
            }
        }

        void TcpConnection::forceClose()
        {
            if(m_state == kConnected || m_state == kDisconnecting)
            {
                setState(kDisconnecting);
                m_loop->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
            }
        }
        void TcpConnection::forceCloseWithDelay(double seconds)
        {
          if(m_state == kConnected || m_state == kDisconnecting)
          {
             setState(kDisconnecting);
             m_loop->runAfter(seconds,std::bind(&TcpConnection::forceClose, shared_from_this())); 
          }
        }

        void TcpConnection::forceCloseInLoop()
        {
            m_loop->assertInLoopThread();
            if(m_state == kConnected || m_state == kDisconnecting)
            {
                handleClose();
            }
        }

        const char* TcpConnection::stateToString() const
        {
             switch(m_state)
             {
                 case kDisconnected:
                     return "kDisconnected";
                 case kConnecting:
                     return "kConnecting";
                 case kConnected:
                     return "kConnected";
                 case kDisconnecting:
                     return "kDisconnecting";
                  default:
                     return "Unknown state";
             }
        }

        void TcpConnection::setTcpNoDelay(bool on)
        {
            m_socket->setTcpNoDelay(on);
        }

        void TcpConnection::startRead()
        {
            m_loop->runInLoop(std::bind(&TcpConnection::startReadInLoop, shared_from_this()));
        }

        void TcpConnection::startReadInLoop()
        {
            m_loop->assertInLoopThread();
            if(!m_reading || !m_Channel->isReading())
            {
                m_Channel->enableReading();
                m_reading = true;
            }
        }

        void TcpConnection::stopRead()
        {
            m_loop->runInLoop(std::bind(&TcpConnection::stopReadInLoop, shared_from_this()));
        }

        void TcpConnection::stopReadInLoop()
        {
            m_loop->assertInLoopThread();
            if(m_reading || m_Channel->isReading())
            {
                m_Channel->disableReading();
                m_reading = false;
            }
        }

        void TcpConnection::connectEstablished()
        {
            m_loop->assertInLoopThread();
            assert(m_state == kConnecting);
            setState(kConnected);
            //m_Channel->tie(shared_from_this());
            m_Channel->enableReading();
            m_connCallback(shared_from_this());
        }

        void TcpConnection::connectDestroyed()
        {
            m_loop->assertInLoopThread();
            if(m_state == kConnected)
            {
                setState(kDisconnected);
                m_Channel->disableAll();
                m_connCallback(shared_from_this());
            }
            m_Channel->remove();
        }

        void TcpConnection::handleRead(std::time_t receiveTime)
        {
            m_loop->assertInLoopThread();
            int saveErrno = 0;
            ssize_t n = m_inputBuffer.readFd(m_Channel->fd(), &saveErrno);
            if(n > 0)
            {
                m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
            }
            else if(n == 0)
            {
                handleClose();
            }
            else
            {
                errno = saveErrno;
                //todo :log
            }
        }

        void TcpConnection::handleWrite()
        {
            m_loop->assertInLoopThread();
            if(m_Channel->isWriting())
            {
                ssize_t n = sockets::write(m_Channel->fd(),
                                        m_outputBuffer.peek(), 
                                         m_outputBuffer.readableBytes());
                if(n > 0)
                {
                    m_outputBuffer.retrieve(n);
                    if(m_outputBuffer.readableBytes() == 0)
                    {
                        m_Channel->disableWriting();
                        if(m_writeCompleteCallback)
                        {
                            m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                        }
                        if(m_state == kDisconnecting)
                        {
                            shutdownInLoop();
                        }
                    }
                }
                else
                {
                    //todo log
                }
            }
            else
            {
                //todo log
            }
        }
       
       void TcpConnection::handleClose()
       {
           m_loop->assertInLoopThread();
           //todo log
           assert(m_state == kConnected || m_state == kDisconnecting);
           setState(kDisconnected);
           m_Channel->disableAll();

           TcpConnection_sPt guardThis(shared_from_this());
           m_connCallback(guardThis);
           m_closeCallback(guardThis);
       }

       void TcpConnection::handleError()
       {
           int err = sockets::getSocketError(m_Channel->fd());
           //todo log
       }


    }
}