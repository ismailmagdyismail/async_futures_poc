#pragma once

//! System Includes
#include <utility>
#include <functional>
#include <memory>

//! Async Engine
#include "RunTime.h"
#include "Socket.h"
#include "SocketReadFuture.h"
#include "Future.h"
#include "TCPHelpers.h"
#include "SocketWriteFuture.h"
#include "SocketConnectFuture.h"

class RunTime;
class TCPServerSocket;

class TCPClientSocket
{
public:
    TCPClientSocket(RunTime *runtime)
    {
        m_pRunTime = runtime;
        m_oSocketInfo = new SocketInfo;
        m_oSocketInfo->fd = socket(AF_INET, SOCK_STREAM, 0);
        MakeNonBlocking(m_oSocketInfo->fd);
    }

    TCPClientSocket(RunTime *runtime, SocketInfo *socketInfo)
    {
        m_pRunTime = runtime;
        m_oSocketInfo = socketInfo;
        MakeNonBlocking(m_oSocketInfo->fd);
    }

    TCPClientSocket(const TCPClientSocket &) = delete;
    TCPClientSocket &operator=(const TCPClientSocket &) = delete;
    TCPClientSocket(TCPClientSocket &&) = delete;
    TCPClientSocket &operator=(TCPClientSocket &&) = delete;

    Future<SocketReadFuture> *Read(char *buffer, unsigned int bufferSize)
    {
        return new Future<SocketReadFuture>(std::in_place, m_pRunTime, buffer, bufferSize, m_oSocketInfo);
    }

    Future<SocketWriteFuture> *Write(char *buffer, unsigned int bufferSize)
    {
        return new Future<SocketWriteFuture>(std::in_place, m_pRunTime, buffer, bufferSize, m_oSocketInfo);
    }

    Future<SocketConnectFuture> *Connect(unsigned int port)
    {
        return new Future<SocketConnectFuture>(std::in_place, m_pRunTime, port, m_oSocketInfo);
    }

    int GetID()
    {
        return m_oSocketInfo->fd;
    }

    void Close()
    {
        close(m_oSocketInfo->fd);
    }

private:
    RunTime *m_pRunTime;
    SocketInfo *m_oSocketInfo;
};