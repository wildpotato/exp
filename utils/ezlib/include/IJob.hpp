#ifndef __EZ_I_JOB_HPP__
#define __EZ_I_JOB_HPP__

#include <memory>
#include <type_traits>
#include "IEvent.hpp"
#include "ReturnCode.hpp"

namespace ez {

class IJob: public IEvent
{
public:
    IJob(int type = 0): IEvent(type) {}

    virtual ~IJob() {}

    virtual ReturnCode DoIt() = 0;
};

using IJobPtr = std::shared_ptr<IJob>;

template <typename JobType, typename ...Args> IJobPtr CreateJob(Args && ...args)
{
    static_assert(std::is_convertible<JobType*, IJob*>::value, "JobType must inherit IJob");

    auto newJobPtr = std::make_shared<JobType>(std::forward<Args>(args)...);
    return std::dynamic_pointer_cast<IJob>(newJobPtr);
}

} // namespace ez

#endif // __EZ_I_JOB_HPP__