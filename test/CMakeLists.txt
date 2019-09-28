cmake_minimum_required(VERSION 2.8)

# compile args
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g ")

# srcs
set (SRCS
     ../poller/defaultpoller.cpp
     ../poller/epollpoller.cpp
     ../util/util_socket.cpp
     ../util/util_socketops.cpp
     ../util/util_inetaddress.cpp
     ../acceptor.cpp
     ../buffer.cpp
     ../channel.cpp
     ../eventloop.cpp
     ../eventloopthread.cpp
     ../eventloopthreadpool.cpp
     ../poller.cpp
     ../tcpconnection.cpp
     ../tcpserver.cpp
     ../timer.cpp
     ../timerqueue.cpp)
# headers

include_directories(../)
include_directories(../poller)
include_directories(../util)


# target
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable(echoServer echoServer.cpp ${SRCS})

# link lib
# LINK_DIRECTORIES() - not use
target_link_libraries(echoServer rt pthread)

