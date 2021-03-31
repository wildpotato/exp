#include "BackgroundWorker.hpp"
#include "IEvent.hpp"

namespace ez {
    
BackgroundWorker::BackgroundWorker()
{
    
}

BackgroundWorker::~BackgroundWorker()
{
    
}

ReturnCode BackgroundWorker::Start(OnJobDoneCB onJobDoneCB)
{
    _onJobDoneCB = onJobDoneCB;
    
    using namespace std::placeholders;
    uv::Loop::OnEventCB eventCB = std::bind(&BackgroundWorker::_onEventCB, this, _1);
    uv::Loop::OnErrorCB errorCB = std::bind(&BackgroundWorker::_onErrorCB, this, _1, _2);
    
    return _loop.Start(eventCB, errorCB);
}

void BackgroundWorker::Stop()
{
    _loop.Stop();
}

ReturnCode BackgroundWorker::AddJob(IJobPtr &&job)
{
    return _loop.SendEvent(std::move(job));
}

void BackgroundWorker::_onEventCB(IEventPtr &eventPtr)
{
    IJobPtr jobPtr = std::dynamic_pointer_cast<IJob>(eventPtr);
    ReturnCode rc = jobPtr->DoIt();
    _onJobDoneCB(jobPtr, rc);
}

BackgroundWorkerPtr CreateBackgroundWorker()
{
    return std::make_unique<BackgroundWorker>();
}

} // namespace ez
