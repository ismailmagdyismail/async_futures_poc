#include <iostream>
#include <set>
#include <vector>

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

class ThenFuture : public IFuture
{
public:
    ThenFuture(IFuture *prev, IFuture *next)
    {
        if (prev == nullptr)
        {
            throw std::runtime_error("[FATAL]: previous future cannot be set to null when chaining futures.");
        }
        m_pPreviousFuture = prev;
        m_pNextFuture = next;
    }

    ThenFuture(const ThenFuture &) = delete;
    ThenFuture(ThenFuture &&) = delete;
    ThenFuture &operator=(const ThenFuture &) = delete;
    ThenFuture &operator=(ThenFuture &&) = delete;

    PollStatus Poll() override
    {
        if (m_pPreviousFuture)
        {
            PollStatus status = m_pPreviousFuture->Poll();
            if (status == PollStatus::Finished)
            {
                m_pPreviousFuture = nullptr;
            }
            if (m_pNextFuture == nullptr)
            {
                return status;
            }
            return PollStatus::Pending;
        }
        else if (m_pNextFuture)
        {
            PollStatus status = m_pNextFuture->Poll();
            return status;
        }
        else
        {
            return PollStatus::Finished;
        }
    }

private:
    IFuture *m_pPreviousFuture;
    IFuture *m_pNextFuture;
};

class IncrementFuture : public IFuture
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

    PollStatus Poll() override
    {
        if (currentIteration < maxIterations)
        {
            std::cerr << currentIteration << std::endl;
            ++currentIteration;
            return PollStatus::Pending;
        }
        if (m_continuation)
            m_continuation();
        return PollStatus::Finished;
    }

    template <typename NextFutureType>
    FutureHandle<NextFutureType> *Then(std::function<NextFutureType *(DataType)> callback)
    {
        auto handle = new FutureHandle<NextFutureType>;
        m_continuation = [cb = std::move(callback), this, handle]()
        {
            auto nextFut = cb(currentIteration);
            handle->Bind(nextFut);
        };
        return handle;
    }

    DataType GetData()
    {
        return currentIteration;
    }

private:
    int maxIterations;
    int currentIteration;
};

class DecrementFuture : public IFuture
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

    PollStatus Poll() override
    {
        if (currentIteration > minIterations)
        {
            std::cerr << currentIteration << std::endl;
            --currentIteration;
            return PollStatus::Pending;
        }
        if (m_continuation)
            m_continuation();
        return PollStatus::Finished;
    }

    template <typename NextFutureType>
    FutureHandle<NextFutureType> *Then(std::function<NextFutureType *(DataType)> callback)
    {
        auto handle = new FutureHandle<NextFutureType>;
        m_continuation = [cb = std::move(callback), this, handle]()
        {
            auto nextFut = cb(currentIteration);
            handle->Bind(nextFut);
        };
        return handle;
    }

    DataType GetData()
    {
        return currentIteration;
    }

private:
    int minIterations;
    int currentIteration;
};

int main()
{

    auto incFuture = new IncrementFuture(10, 0);

    incFuture->Then<DecrementFuture>([](int data)
                                     {
        std::cerr <<"Incerement future Counter value reached " << std::endl;
        auto fut =  new DecrementFuture(0, data); 
        return fut; });

    std::cerr << "Futures registered with runtime = " << runtime.size() << std::endl;
    std::set<int> removedIndecies;
    while (removedIndecies.size() < runtime.size())
    {
        for (int i = 0.; i < runtime.size(); ++i)
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