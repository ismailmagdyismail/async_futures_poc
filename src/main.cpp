#include <iostream>

enum class PollStatus
{
    Pending,
    Finished,
    Error,
};

class IFuture
{
public:
    virtual PollStatus Poll() = 0;
    virtual ~IFuture() = default;
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
        return PollStatus::Finished;
    }

private:
    int maxIterations;
    int currentIteration;
};

class DecrementFuture : public IFuture
{
public:
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
        return PollStatus::Finished;
    }

private:
    int minIterations;
    int currentIteration;
};

int main()
{
    std::vector<std::unique_ptr<IFuture>> vecFutures;
    auto incFuture = std::make_unique<IncrementFuture>(10, 0);
    auto DecFuture = std::make_unique<DecrementFuture>(0, 10);

    // vecFutures.push_back(std::move(DecFuture));
    // vecFutures.push_back(std::move(DecFuture));

    auto thenFuture = std::make_unique<ThenFuture>(incFuture.release(), DecFuture.release());

    vecFutures.push_back(std::move(thenFuture));

    while (!vecFutures.empty())
    {
        for (auto it = vecFutures.begin(); it != vecFutures.end();)
        {
            auto &ptr = *it;
            auto pollResult = ptr->Poll();
            if (pollResult == PollStatus::Finished || pollResult == PollStatus::Error)
            {
                it = vecFutures.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}