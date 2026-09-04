#pragma once

#include <functional>
#include "RunTime.h"

enum class PollStatus
{
    Pending,
    Finished,
    Error,
};

class IFuture
{
public:
    IFuture(RunTime *runtime)
    {
        runtime->AddFuture(this);
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