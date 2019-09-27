cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 ")

set (SRCS
     poller/defaultpoller.cpp
     poller/epollpoller.cpp
     util/util_socket.cpp
     util/util_socketops.cpp
     acceptor.cpp
     buffer.cpp
     channel.cpp
     eventloop.cpp
     eventloopthread.cpp
     eventloopthreadpool.cpp
     poller.cpp
     tcpconnection.cpp
     tcpserver.cpp
     timer.cpp
     timerqueue.cpp)
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_library( seenet ${SRCS})

target_link_libraries(seenet rt)

install(TARGETS seenet DESTINATION lib)

include_directories(/usr/inclue)
set (HEADERS
    poller/epollpoller.h
    util/util_socket.h
    util/util_endian.h
    util/util_socketops.h
    acceptor.h
    buffer.h
    channel.h
    copyable.h
    eventloop.h
    eventloopthread.h
    eventloopthreadpool.h
    noncopyable.h
    poller.h
    seenet.h
    tcpconnection.h
    tcpserver.h
    timer.h
    timerqueue.h
    timerid.h)
install (FILES ${HEADERS} DESTINATION include/seenet/)
