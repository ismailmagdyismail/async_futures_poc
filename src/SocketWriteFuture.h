#pragma once
#include "IFuture.h"
#include "Socket.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

class SocketWriteFuture
{
public:
    using DataType = int;

    SocketWriteFuture(char *buffer, unsigned int bufferSize, SocketInfo *pSocketInfo) : m_pSocketInfo(pSocketInfo)
    {
        m_writeBuffer = buffer;
        m_uiWriteBufferSize = bufferSize;
    }

    PollStatus Poll()
    {
        int iWrittenBytes = write(m_pSocketInfo->fd, m_writeBuffer, m_uiWriteBufferSize);
        std::cerr << "written bytes "<<" on fd" << m_pSocketInfo->fd<<", " << iWrittenBytes << ", " << errno << std::endl;
        if (iWrittenBytes == -1)
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
        else
        {
            writtenBytes += iWrittenBytes;
            if (writtenBytes == m_uiWriteBufferSize)
            {
                return PollStatus::Finished;
            }
            return PollStatus::Pending;
        }
    }

    DataType GetData()
    {
        return writtenBytes;
    }

private:
    SocketInfo *m_pSocketInfo;
    char *m_writeBuffer;
    unsigned int m_uiWriteBufferSize;
    int writtenBytes = 0;
};
