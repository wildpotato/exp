#include "uv/Loop.hpp"
#include <chrono>

namespace ez { namespace uv {

Loop::Loop()
{

}

Loop::~Loop()
{
    Stop();
}

ReturnCode Loop::Start(OnEventCB onEventCB, OnErrorCB onErrorCB)
{
    ReturnCode rc = _Init(onEventCB, onErrorCB);
    if (!rc)
        return rc;
    
    // start thread
    _thread = std::move(std::thread(
                [this] ()
                {
                    uv_update_time(&_uvloop);
                    
                    int status = uv_run(&_uvloop, UV_RUN_DEFAULT);

                    // still have active handles
                    if (status != 0)
                    {
                        // close all handles in loop not been closed
                        uv_walk(&_uvloop,
                                [] (uv_handle_t* handle, void* arg)
                                {
                                    uv_close(handle, nullptr);
                                },
                                nullptr);

                        // run again to let handle's close callback been called
                        uv_run(&_uvloop, UV_RUN_NOWAIT);
                    }

                    uv_loop_close(&_uvloop);
                    
                    uv_loop_set_data(&_uvloop, nullptr);
                    uv_handle_set_data((uv_handle_t *)&_uvasync, nullptr);
                }));

    return {};
}

void Loop::Stop()
{
    if (uv_handle_get_data((uv_handle_t *)&_uvasync) == this)
    {
        std::call_once(_stopOnceFlag, 
                [this] ()
                {
                    // send  event to trigger close
                    SendEvent(CreateEvent<IEvent>());    
                    
                    if (_thread.joinable())
                        _thread.join();
                });
    }
}

ReturnCode Loop::SendEvent(const IEventPtr &eventPtr)
{
    _eventQueue.Push(eventPtr);
    int status = uv_async_send(&_uvasync);
    return (status == 0) ? ReturnCode() : ReturnCode(-1, status);
}
ReturnCode Loop::SendEvent(IEventPtr &&eventPtr)
{
    _eventQueue.Push(std::move(eventPtr));
    int status = uv_async_send(&_uvasync);
    return (status == 0) ? ReturnCode() : ReturnCode(-1, status);    
}

ReturnCode Loop::StartTimer(const TimerPtr &timer, int timeoutMs, int repeatMs)
{
    IEventPtr event = CreateEvent<StartTimerEvent>(timer, timeoutMs, repeatMs);
    return SendEvent(std::move(event));    
}

ReturnCode Loop::StopTimer(const TimerPtr &timer)
{
    timer->Disable();
    IEventPtr event = CreateEvent<StopTimerEvent>(timer);
    return SendEvent(std::move(event));
}

ReturnCode Loop::_Init(OnEventCB onEventCB, OnErrorCB onErrorCB)
{
    // check not inited before
    if (uv_loop_get_data(&_uvloop) == this)
        return ReturnCode(-1, -1);

    if (uv_handle_get_data((uv_handle_t *)&_uvasync) == this)
        return ReturnCode(-1, -2);

    // init uv loop
    int status = uv_loop_init(&_uvloop);
    if (status != 0)
        return ReturnCode(-2, status);

    // init async manager
    status = uv_async_init(&_uvloop, &_uvasync,
                    [] (uv_async_t* handle)
                    {
                        Loop *loop = (Loop *)uv_handle_get_data((uv_handle_t *)handle);
                        loop->_HandleEvents();
                    });

    if (status != 0)
    {
        uv_loop_close(&_uvloop);
        return ReturnCode(-3, status);
    }

    uv_loop_set_data(&_uvloop, this);
    uv_handle_set_data((uv_handle_t *)&_uvasync, this);
    _onEventCB = onEventCB;
    _onErrorCB= onErrorCB;

    return {};
}

void Loop::_HandleEvents()
{
    using namespace std::placeholders;
    
    IEventPtr event;
    while (_eventQueue.TryPop(event))
    {
        int type = event->GetType();
        if (type >= 0)
        {
            _onEventCB(event);
        }
        else
        {
            switch (type)
            {

            case (int)ELoopError::StartTimer:
            {
                StartTimerEvent *realEvent = (StartTimerEvent *)event.get();
                ReturnCode rc = realEvent->_timer->_Start(&_uvloop, realEvent->_timeoutMs, realEvent->_repeatMs);
                if (!rc)
                    _onErrorCB(ELoopError::StartTimer, "start fail");
            }
                break;
            case (int)ELoopError::StopTimer:
            {
                StopTimerEvent *realEvent = (StopTimerEvent *)event.get();
                ReturnCode rc = realEvent->_timer->_Stop();
                if (!rc)
                    _onErrorCB(ELoopError::StopTimer, "stop fail");
            }
                break;
            default:
                uv_stop(&_uvloop);
            }
        }
            
    }
}

} } // namespace ez::uv
