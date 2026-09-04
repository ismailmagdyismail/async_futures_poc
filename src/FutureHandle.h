#pragma once

#include <functional>

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
