#pragma once

//! System includes
#include <type_traits>

//! Future includes
#include "IFuture.h"
#include "FutureHandle.h"

template <typename FutureWrappedType>
class Future : public IFuture
{
public:
    //! Emplace like API
    //! to avoid copying or un-necessary pointers, heap allocations
    template <typename... Args>
    explicit Future(std::in_place_t, RunTime *runtime, Args &&...args)
        : IFuture(runtime), m_oWrappedFuture(std::forward<Args>(args)...)
    {
    }

    using DataType = typename FutureWrappedType::DataType;

    PollStatus Poll() override
    {
        PollStatus eStatus = m_oWrappedFuture.Poll();
        if (eStatus == PollStatus::Finished)
        {
            if (m_continuation)
            {
                m_continuation();
            }
        }
        return eStatus;
    }

    template <typename Callback>
    auto Then(Callback &&callback)
    {
        using NextFutureType = std::remove_pointer_t<std::invoke_result_t<Callback &, DataType>>;
        auto handle = new FutureHandle<NextFutureType>;
        m_continuation =[cb = std::forward<Callback>(callback), this, handle]() mutable
        {
            auto nextFut = std::invoke(cb, GetData());
            handle->Bind(nextFut);
        };

        return handle;
    }

    DataType GetData()
    {
        return m_oWrappedFuture.GetData();
    }

private:
    FutureWrappedType m_oWrappedFuture;
};
