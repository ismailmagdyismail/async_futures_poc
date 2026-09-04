#include "RunTime.h"
#include "IFuture.h"

void RunTime::AddFuture(IFuture *future)
{
    runtime.push_back(future);
}

void RunTime::Stop()
{
    m_bIsRunning = false;
}

void RunTime::Run()
{
    while (m_bIsRunning)
    {
        std::set<int> removedIndecies;
        for (unsigned int i = 0; i < runtime.size(); ++i)
        {
            auto pollResult = runtime[i]->Poll();
            if (pollResult == PollStatus::Finished || pollResult == PollStatus::Error)
            {
                removedIndecies.insert(i);
            }
        }
        for (auto it = removedIndecies.rbegin(); it != removedIndecies.rend(); ++it)
        {
            auto index = *it;
            delete runtime[index];
            runtime.erase(runtime.begin() + index);
        }
        removedIndecies.clear();
    }
}