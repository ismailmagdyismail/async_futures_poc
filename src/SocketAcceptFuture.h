#include "IFuture.h"
#include "Socket.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

class SocketAcceptFuture
{
public:
    using DataType = SocketInfo *;

    SocketAcceptFuture(SocketInfo *pSocketInfo) : m_pSocketInfo(pSocketInfo)
    {
    }

    PollStatus Poll()
    {
        int clientFd = accept(m_pSocketInfo->fd, nullptr, nullptr);
        if (clientFd == -1)
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
            m_pCreatedClientSocketInfo = new SocketInfo{clientFd};
            return PollStatus::Finished;
        }
    }

    DataType GetData()
    {
        return m_pCreatedClientSocketInfo;
    }

private:
    SocketInfo *m_pSocketInfo;
    SocketInfo *m_pCreatedClientSocketInfo{nullptr};
};
