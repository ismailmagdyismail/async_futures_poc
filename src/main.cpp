#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

enum class PollStatus
{
    Pending,
    Finished,
    Error,
};

class IFuture;
std::vector<IFuture *> runtime;

class IFuture
{
public:
    IFuture()
    {
        runtime.push_back(this);
    };

    virtual PollStatus Poll() = 0;
    virtual ~IFuture() = default;
    void SetContinuation(std::function<void(void)> cb)
    {
        m_continuation = std::move(cb);
    }

protected:
    std::function<void(void)> m_continuation;
};

template <typename BoundFutureType>
class FutureHandle
{
public:
    template <typename NextFutureType>
    FutureHandle<NextFutureType> *Then(std::function<NextFutureType *(typename BoundFutureType::DataType)> callback)
    {
        if (boundFut)
        {
            return boundFut->Then(std::move(callback));
        }
        auto handle = new FutureHandle<NextFutureType>;
        continuation = [cb = std::move(callback), this, handle]()
        {
            if (boundFut == nullptr)
            {
                throw std::runtime_error("[FATAL]: future handle not bound in continuation cb");
            }
            typename BoundFutureType::DataType data = boundFut->GetData();
            auto nextFut = cb(data);
            handle->Bind(nextFut);
        };
        return handle;
    }

    void Bind(BoundFutureType *fut)
    {
        if (boundFut != nullptr)
        {
            throw std::runtime_error("[FATAL]: bound twice");
        }
        boundFut = fut;
        if (continuation)
        {
            boundFut->SetContinuation(std::move(continuation));
        }
    }

private:
    BoundFutureType *boundFut{nullptr};
    std::function<void(void)> continuation;
};

template <typename FutureWrappedType>
class Future : public IFuture
{
public:
    Future(FutureWrappedType wrappedFuture) : m_oWrappedFuture(wrappedFuture)
    {
    }

    using DataType = typename FutureWrappedType::DataType;

    PollStatus Poll() override
    {
        PollStatus eStatus = m_oWrappedFuture.Poll();
        if (eStatus == PollStatus::Finished)
        {
            if (m_continuation)
            {
                m_continuation();
            }
        }
        return eStatus;
    }

    template <typename NextFutureType>
    FutureHandle<NextFutureType> *Then(std::function<NextFutureType *(DataType)> callback)
    {
        auto handle = new FutureHandle<NextFutureType>;
        m_continuation = [cb = std::move(callback), this, handle]()
        {
            auto nextFut = cb(GetData());
            handle->Bind(nextFut);
        };
        return handle;
    }

    DataType GetData()
    {
        return m_oWrappedFuture.GetData();
    }

private:
    FutureWrappedType m_oWrappedFuture;
};

class IncrementFuture
{
public:
    using DataType = int;

    IncrementFuture(int n, int i) : maxIterations(n),
                                    currentIteration(i)
    {
        if (maxIterations < currentIteration)
        {
            throw std::runtime_error("[FATAL]: max iterations cannot be less than current iteration in counter future");
        }
    }

    PollStatus Poll()
    {
        if (currentIteration < maxIterations)
        {
            std::cerr << currentIteration << std::endl;
            ++currentIteration;
            return PollStatus::Pending;
        }
        return PollStatus::Finished;
    }

    DataType GetData()
    {
        return currentIteration;
    }

private:
    int maxIterations;
    int currentIteration;
};

class DecrementFuture
{
public:
    using DataType = int;

    DecrementFuture(int n, int i) : minIterations(n),
                                    currentIteration(i)
    {
        if (minIterations > currentIteration)
        {
            throw std::runtime_error("[FATAL]: min iterations cannot be more than current iteration in counter future");
        }
    }

    PollStatus Poll()
    {
        if (currentIteration > minIterations)
        {
            std::cerr << currentIteration << std::endl;
            --currentIteration;
            return PollStatus::Pending;
        }
        return PollStatus::Finished;
    }

    DataType GetData()
    {
        return currentIteration;
    }

private:
    int minIterations;
    int currentIteration;
};

struct SocketInfo
{
    int fd;
};

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

class DoneFuture
{
public:
    using DataType = bool;

    PollStatus Poll()
    {
        return PollStatus::Finished;
    }

    bool GetData()
    {
        return true;
    }
};

int main()
{
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(serverFD, F_SETFL, O_NONBLOCK);
    sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    address.sin_family = AF_INET;

    int bindStatus = bind(serverFD, reinterpret_cast<sockaddr *>(&address), sizeof(address));
    if (bindStatus == -1)
    {
        std::cerr << "Error binding socket: with code " << errno << " and message: " << strerror(errno) << std::endl;
        return 1;
    }
    int listenStatus = listen(serverFD, 0);
    if (listenStatus != 0)
    {
        std::cerr << "Error Listening on Server Socket with port 8080" << std::endl;
        return 1;
    }

    Future<SocketAcceptFuture> *acceptFuture = new Future<SocketAcceptFuture>(SocketAcceptFuture(new SocketInfo{serverFD}));

    acceptFuture->Then<Future<SocketReadFuture>>([](SocketInfo *clientSocketInfo)
                                                 {
        std::cerr << "Socket accepted, fd: " << clientSocketInfo->fd << std::endl;
        return new Future<SocketReadFuture>(SocketReadFuture(1024, clientSocketInfo)); })
        ->Then<Future<DoneFuture>>([](SocketReadFuture::ReadResult readResult)
                                   {
        std::cerr << "Socket read completed, bytes read: " << readResult.bytesRead << std::endl;
        std::cerr <<"Read bytes "<< std::string_view(readResult.buffer, readResult.bytesRead) << std::endl;
        return new Future<DoneFuture>(DoneFuture()); });

    Future<DecrementFuture> *decrementFuture = new Future<DecrementFuture>(DecrementFuture(0, 10));
    Future<IncrementFuture> *incrementFuture2 = new Future<IncrementFuture>(IncrementFuture(10, 0));

    decrementFuture->Then<Future<DoneFuture>>([](DecrementFuture::DataType value)
                                              { std::cerr << " Decrement future completed with value: " << value << std::endl;
                        return new Future<DoneFuture>(DoneFuture()); });
    incrementFuture2->Then<Future<DoneFuture>>([](IncrementFuture::DataType value)
                                               { std::cerr << " Increment future completed with value: " << value << std::endl;
                        return new Future<DoneFuture>(DoneFuture()); });

    std::cerr << "Futures registered with runtime = " << runtime.size() << std::endl;
    std::set<int> removedIndecies;
    while (removedIndecies.size() < runtime.size())
    {
        for (unsigned int i = 0; i < runtime.size(); ++i)
        {
            if (removedIndecies.find(i) != removedIndecies.end())
            {
                continue;
            }
            auto pollResult = runtime[i]->Poll();
            if (pollResult == PollStatus::Finished || pollResult == PollStatus::Error)
            {
                std::cerr << "future removed " << std::endl;
                removedIndecies.insert(i);
            }
        }
    }
}