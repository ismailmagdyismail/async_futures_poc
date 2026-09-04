#pragma once
#include "IFuture.h"
#include "Socket.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

class SocketReadFuture
{
public:
    SocketReadFuture(unsigned int bufferSize, SocketInfo *pSocketInfo) : m_pSocketInfo(pSocketInfo)
    {
        m_readResult.buffer = new char[bufferSize];
        m_bufferSize = bufferSize;
    }
    struct ReadResult
    {
        int bytesRead;
        char *buffer{nullptr};
    };

    using DataType = ReadResult;

    PollStatus Poll()
    {
        int iBytesRead = read(m_pSocketInfo->fd, m_readResult.buffer, m_bufferSize);
        if (iBytesRead == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return PollStatus::Pending;
            }
            else
            {
                return PollStatus::Error;
            }
        }
        m_readResult.bytesRead = iBytesRead;
        return PollStatus::Finished;
    }

    DataType GetData()
    {
        return m_readResult;
    }

private:
    SocketInfo *m_pSocketInfo;
    unsigned int m_bufferSize;
    ReadResult m_readResult;
};
