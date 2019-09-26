#include"seenet.h"
#include"noncopyable.h"
#include"tcpserver.h"
#include"./util/util_inetaddress.h"
#include"acceptor.h"
#include"eventLoopThreadPool.h"
#include"eventloop.h"
#include"tcpconnection.h"
#include"./util/util_socketops.h"
#include"./util/util_socket.h"

#include<string>

namespace seenet{
    namespace net{
        TcpServer::TcpServer(EventLoop_sPt loop, 
                             const InetAddress& listenAddr, 
                             const std::string& nameArg,
                             Option option):m_loop(loop), m_ipPort(listenAddr.toIpPort()),m_name(nameArg),
                             m_acceptor(new Acceptor(loop, listenAddr, option == kReusePort)),
                             m_threadPool(new EventLoopThreadPool(loop, nameArg)),
                             m_connectionCallback(defaultConnectionCallback), 
                             m_messageCallback(defaultMessageCallback), 
                             m_nextConnID(1),
                             m_started(0)

        {
            m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
        }

        TcpServer::~TcpServer()
        {
            m_loop->assertInLoopThread();
            //todo log

            for(auto& item: m_connections)
            {
                TcpConnection_sPt conn(item.second);
                item.second.reset();
                conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
            }
        }

        void TcpServer::setThreadNum(int numThreads)
        {
            assert(0 <= numThreads);
            m_threadPool->setThreadNum(numThreads);
        }

        void TcpServer::start()
        {
            if(m_started.fetch_add(1) == 0)
            {
               m_threadPool->start(m_threadInitCallback);

               assert(!m_acceptor->listening());
               m_loop->runInLoop(std::bind(std::bind(&Acceptor::listen, m_acceptor.get())));
            }
        }

        void TcpServer::newConnection(int sockFd, const InetAddress& peeraddr)
        {
            m_loop->assertInLoopThread();
            EventLoop_sPt ioLoop = m_threadPool->getNextLoop();

            char buf[64];
            snprintf(buf, sizeof(buf), "-%s#%d", m_ipPort.c_str(), m_nextConnID);
            ++m_nextConnID;
            std::string connName = m_name + buf;
            //todo log
            InetAddress localAddr(sockets::getLocalAddr(sockFd));

            TcpConnection_sPt conn(new TcpConnection(ioLoop,
                                   connName, sockFd, localAddr, peeraddr));
            m_connections[connName] = conn;
            conn->setNewConnectionCallback(m_connectionCallback);
            conn->setMessageCallback(m_messageCallback);
            conn->setWriteCompleteCallback(m_writeCompleteCallback);
            conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this,
                                   std::placeholders::_1));
            ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
        }

        void TcpServer::removeConnection(const TcpConnection_sPt& conn)
        {
            m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
        }

        void TcpServer::removeConnectionInLoop(const TcpConnection_sPt& conn)
        {
            m_loop->assertInLoopThread();
            //todo log
            size_t n = m_connections.erase(conn->name());
            
            EventLoop_sPt ioLoop = conn->getLoop();
            ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
        }
    }
}