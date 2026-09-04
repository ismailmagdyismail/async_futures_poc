#pragma once

//! System Includes
#include <utility>
#include <arpa/inet.h>
#include <functional>
#include <unistd.h>

//! Async Future Includes
#include "RunTime.h"
#include "Socket.h"
#include "TCPHelpers.h"
#include "SocketAcceptFuture.h"
#include "Future.h"

class TCPServerSocket
{
public:
    TCPServerSocket(RunTime *runtime)
    {
        m_pRunTime = runtime;
        m_oSocketData.fd = socket(AF_INET, SOCK_STREAM, 0);
        MakeNonBlocking(m_oSocketData.fd);
    }

    TCPServerSocket(const TCPServerSocket &) = delete;
    TCPServerSocket &operator=(const TCPServerSocket &) = delete;
    TCPServerSocket(TCPServerSocket &&) = delete;
    TCPServerSocket &operator=(TCPServerSocket &&) = delete;

    bool Listen(unsigned int port)
    {
        auto address = CreateLocalAddress(port);
        int bindStatus = bind(m_oSocketData.fd, reinterpret_cast<sockaddr *>(&address), sizeof(address));
        if (bindStatus == -1)
        {
            return false;
        }
        int listenStatus = listen(m_oSocketData.fd, SOMAXCONN);
        return listenStatus == 0;
    }

    int GetID()
    {
        return m_oSocketData.fd;
    }

    void Close()
    {
        close(m_oSocketData.fd);
    }


    Future<SocketAcceptFuture> *Accept()
    {
        return new Future<SocketAcceptFuture>(std::in_place, m_pRunTime, &m_oSocketData);
    }

private:
    SocketInfo m_oSocketData;
    RunTime *m_pRunTime;
};