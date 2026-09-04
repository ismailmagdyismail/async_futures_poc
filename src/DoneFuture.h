#pragma once
#include "IFuture.h"

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