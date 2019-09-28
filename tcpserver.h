#ifndef _SEENET_TCPSERVER_H
#define _SEENET_TCPSERVER_H

#include "seenet.h"
#include "noncopyable.h"
#include "./util/util_inetaddress.h"
#include "eventloopthread.h"
#include "eventloopthreadPool.h"

#include <map>
#include <atomic>
#include <memory>

namespace seenet{
    namespace net{
        class Acceptor;
        class EventLoop;
        class EventLoopThreadPool;

        class TcpServer:public NonCopyable
        {
        public:
            using ConnectionMap = std::map<std::string, TcpConnection_sPt>;

            enum Option
            {
               kNoReusePort,
               kReusePort,
            };

            TcpServer(EventLoop_sPt loop,
                      const InetAddress& listenAddr,
                      const std::string& nameArg,
                      Option option = kNoReusePort);
            ~TcpServer();

            const std::string& ipPort() const { return m_ipPort;}
            const std::string& name() const { return m_name;}
            EventLoop_sPt getLoop() const {return m_loop;}

            void setThreadNum(int numThreads);
            void setThreadInitCallback(const ThreadInitCallback& cb)
            { m_threadInitCallback = cb;}
            std::shared_ptr<EventLoopThreadPool> threadPool()
            {return m_threadPool;}

            void start();

            void setNewConnectionCallback(const ConnectionCallback& cb)
            { m_connectionCallback = cb;}
            void setMessageCallback(const MessageCallback& cb)
            { m_messageCallback = cb;}
            void setWriteCompleteCallback(const WriteCompleteCallback& cb)
            { m_writeCompleteCallback = cb;}

        private:
            void newConnection(int sockFd, const InetAddress& peerAddress);
            void removeConnection(const TcpConnection_sPt& conn);
            void removeConnectionInLoop(const TcpConnection_sPt& conn);

            EventLoop_sPt m_loop;
            const std::string m_ipPort;
            const std::string m_name;
            std::shared_ptr<Acceptor> m_acceptor;
            std::shared_ptr<EventLoopThreadPool> m_threadPool;

            ConnectionCallback m_connectionCallback;
            MessageCallback m_messageCallback;
            WriteCompleteCallback m_writeCompleteCallback;
            ThreadInitCallback m_threadInitCallback;
            std::atomic<int32_t> m_started;

            int m_nextConnID;
            ConnectionMap m_connections;
        };

    }
}

#endif