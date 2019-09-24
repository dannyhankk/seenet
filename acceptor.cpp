#include "seenet.h"
#include "acceptor.h"
#include "channel.h"
#include "eventloop.h"
#include "util/util_socket.h"
#include "util/util_inetaddress.h"
#include "util/util_socketops.h"

#include <errno.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <unistd.h>
#include <assert.h>

namespace seenet{
    namespace net{
        Acceptor::Acceptor(EventLoop_sPt loop, const InetAddress& ListenAddr, bool reuseport)
        :m_loop(loop), 
        m_acceptSocket(sockets::createNonblockingOrDie(ListenAddr.family())),
        m_acceptChannel(new Channel(loop, m_acceptSocket.fd())),
        m_bListening(false),
        m_idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
        {
            assert(m_idleFd >= 0);
            m_acceptSocket.setReuseAddr(true);
            m_acceptSocket.setReusePort(reuseport);
            m_acceptSocket.bindAddress(ListenAddr);
            m_acceptChannel->setReadCallback(std::bind(&Acceptor::handleRead, shared_from_this()));
        }

        Acceptor::~Acceptor()
        {
            m_acceptChannel->disableAll();
            m_acceptChannel->remove();
            ::close(m_idleFd);
        }

        void Acceptor::listen()
        {
            m_loop->assertInLoopThread();
            m_bListening = true;
            m_acceptSocket.listen();
            m_acceptChannel->enableReading();
        }

        void Acceptor::handleRead()
        {
            m_loop->assertInLoopThread();
            InetAddress peeraddr;
            int connFd = m_acceptSocket.accept(&peeraddr);
            if(connFd >= 0)
            {
                if(m_newConnectionCallback)
                {
                   m_newConnectionCallback(connFd, peeraddr);
                }
                else 
                {
                   sockets::close(connFd);
                }
            }
            else
            {
                //log syserr
                if(errno == EMFILE)
                {
                    ::close(m_idleFd);
                    m_idleFd = ::accept(m_acceptSocket.fd(), NULL, NULL);
                    ::close(m_idleFd);
                    m_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
                }
            }
        }
    }
}