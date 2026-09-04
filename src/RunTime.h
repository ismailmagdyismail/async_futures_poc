#pragma once

#include <set>

class IFuture;

class RunTime
{
public:
    void AddFuture(IFuture *fut);
    void Stop();
    void Run();

private:
    bool m_bIsRunning{true};
    std::vector<IFuture *> runtime;
};