#pragma once
#include "IFuture.h"
#include <iostream>

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
