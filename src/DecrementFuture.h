#pragma once

#include "IFuture.h"
#include <iostream>

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