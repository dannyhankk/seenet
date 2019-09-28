#include "../seenet.h"
#include"../tcpserver.h"
#include "../eventloop.h"
#include "../util/util_inetaddress.h"
#include "../tcpconnection.h"

#include<functional>
#include<memory>
#include<iostream>

using namespace seenet;
using namespace seenet::net;


int main()
{
    EventLoop_sPt sLoop(new EventLoop());
    std::string serverName("echoServer");
    InetAddress addr("127.0.0.1", 8866);
    TcpServer svr(sLoop, addr, serverName);

    svr.setNewConnectionCallback([](TcpConnection_sPt conn){
        if(conn.get())
        {
            std::cout << "new connection comming" << std::endl;
            return ;
        }

    });

    svr.setMessageCallback([](TcpConnection_sPt conn ,Buffer* buf, std::time_t time){
         if(conn.get())
         {
             std::cout << "time: " << time << std::endl;
             std::cout << "Received message: " << buf->peek() << std::endl;
             conn->send((const void *)buf->peek(), buf->readableBytes());
         }
    });

    svr.setWriteCompleteCallback([](TcpConnection_sPt conn){
        if(conn.get())
        {
            std::cout << "message sended" << std::endl;
        }
    });

    svr.setThreadNum(8);
   /* svr.setThreadInitCallback([](EventLoop_sPt loop){
       std::cout << "Thread start" << std::endl;
    });*/
    svr.start();
    sLoop->loop();

    int in;
    std::cin >>in;
    sLoop->quit();
}



