#ifndef __EZ_BACKGROUND_WORKER_HPP__
#define __EZ_BACKGROUND_WORKER_HPP__

#include <memory>
#include "IJob.hpp"
#include "uv/Loop.hpp"

namespace ez {

class BackgroundWorker
{
public:
    typedef std::function<void(IJobPtr &job, ReturnCode rc)> OnJobDoneCB;
    
    BackgroundWorker();
    ~BackgroundWorker();
    
    ReturnCode Start(OnJobDoneCB onJobDoneCB);

    // this will block until all job done, you should not call this inside OnJobDoneCB
    void Stop();
    
    ReturnCode AddJob(IJobPtr &&job);

private:
    
    void _onEventCB(IEventPtr &eventPtr);
    
    void _onErrorCB(uv::ELoopError rc, const std::string &msg) {}

    uv::Loop _loop;
    OnJobDoneCB _onJobDoneCB;
};

using BackgroundWorkerPtr = std::unique_ptr<BackgroundWorker>;

BackgroundWorkerPtr CreateBackgroundWorker();

} // namespace ez

#endif // __EZ_BACKGROUND_WORKER_HPP__
