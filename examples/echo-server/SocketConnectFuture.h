#pragma once
#include "IFuture.h"
#include "Socket.h"
#include "TCPHelpers.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <iostream>

class SocketConnectFuture
{
public:
    using DataType = bool;

    SocketConnectFuture(unsigned int port, SocketInfo *pSocketInfo) : m_pSocketInfo(pSocketInfo)
    {
        m_iPort = port;
    }

    PollStatus Poll()
    {
        auto address = CreateLocalAddress(m_iPort);
        int connectStatus = connect(m_pSocketInfo->fd, reinterpret_cast<sockaddr *>(&address), sizeof(address));
        if (connectStatus == 0)
            return PollStatus::Finished;
        if(errno == EISCONN)
            return PollStatus::Finished;
        if (errno == EINPROGRESS || errno == EALREADY)
            return PollStatus::Pending;

        std::cerr << "Error on connect " << errno << std::endl;
        return PollStatus::Error;
    }

    DataType GetData()
    {
        return true;
    }

private:
    SocketInfo *m_pSocketInfo;
    unsigned int m_iPort;
};
