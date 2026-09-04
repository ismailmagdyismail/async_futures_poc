#pragma once

//! System includes
#include <type_traits>
#include <functional>

template <typename BoundFutureType>
class FutureHandle
{
public:
    template <typename Callback>
    auto Then(Callback &&callback)
    {
        using NextFutureType = std::remove_pointer_t<std::invoke_result_t<Callback &, typename BoundFutureType::DataType>>;
        if (boundFut)
        {
            return boundFut->Then(std::forward<Callback>(callback));
        }
        auto handle = new FutureHandle<NextFutureType>;
        continuation = [cb = std::forward<Callback>(callback), this, handle]() mutable
        {
            if (boundFut == nullptr)
            {
                throw std::runtime_error("[FATAL]: future handle not bound in continuation cb");
            }
            auto data = boundFut->GetData();
            auto nextFut = std::invoke(cb, data);
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
