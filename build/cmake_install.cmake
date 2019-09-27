# Install script for directory: /mnt/WorkDir/seenet

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/mnt/WorkDir/seenet/build/lib/libseenet.a")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/seenet" TYPE FILE FILES
    "/mnt/WorkDir/seenet/epollpoller.h"
    "/mnt/WorkDir/seenet/util/util_socket.h"
    "/mnt/WorkDir/seenet/util/util_endian.h"
    "/mnt/WorkDir/seenet/util/util_socketops.h"
    "/mnt/WorkDir/seenet/acceptor.h"
    "/mnt/WorkDir/seenet/buffer.h"
    "/mnt/WorkDir/seenet/channel.h"
    "/mnt/WorkDir/seenet/copyable.h"
    "/mnt/WorkDir/seenet/eventloop.h"
    "/mnt/WorkDir/seenet/eventloopthread.h"
    "/mnt/WorkDir/seenet/eventloopthreadpool.h"
    "/mnt/WorkDir/seenet/noncopyable.h"
    "/mnt/WorkDir/seenet/poller.h"
    "/mnt/WorkDir/seenet/seenet.h"
    "/mnt/WorkDir/seenet/tcpconnection.h"
    "/mnt/WorkDir/seenet/tcpserver.h"
    "/mnt/WorkDir/seenet/timer.h"
    "/mnt/WorkDir/seenet/timerqueue.h"
    "/mnt/WorkDir/seenet/timerid.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/mnt/WorkDir/seenet/build/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/mnt/WorkDir/seenet/build/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
