#ifndef __EZ_UV_LOOP_HPP__
#define __EZ_UV_LOOP_HPP__

#include <thread>
#include <functional>
#include "uv/Timer.hpp"
#include "LockQueue.hpp"
#include "IEvent.hpp"

namespace ez { namespace uv {

enum class ELoopError
{
    StartTimer = -100,
    StopTimer = -101,
};

class Loop
{
public:
    typedef std::function<void(IEventPtr &eventPtr)> OnEventCB;
    typedef std::function<void(ELoopError rc, const std::string &msg)> OnErrorCB;

    Loop();
    ~Loop();

    // prevent copy/assignment/move, because uv loop is not safe to move while running
    Loop(const Loop &loop) = delete;
    Loop(Loop &&loop) = delete;
    Loop &operator=(const Loop &loop) = delete;
    Loop &operator=(Loop &&loop) = delete;

    uv_loop_t *GetHandle() { return &_uvloop; }

    ReturnCode Start(OnEventCB onEventCB, OnErrorCB onErrorCB);

    // this will block until loop finish, should not be called inside callback
    void Stop();

    ReturnCode SendEvent(const IEventPtr &eventPtr);
    ReturnCode SendEvent(IEventPtr &&eventPtr);
    
    // start a timer, callback will be called repeatedly until stopped
    ReturnCode StartTimer(const TimerPtr &timer, int timeoutMs, int repeatMs);

    ReturnCode StopTimer(const TimerPtr &timer);
    
private:

    ReturnCode _Init(OnEventCB onEventCB, OnErrorCB onErrorCB);
    
    void _HandleEvents();
    
    void _OnTimerTimeout(IEventPtr eventPtr, uint64_t loopTime);
    
    std::once_flag _stopOnceFlag;
    std::thread _thread;
    uv_loop_t _uvloop;
    uv_async_t _uvasync;
    OnEventCB _onEventCB;
    OnErrorCB _onErrorCB;
    ez::LockQueue<IEventPtr> _eventQueue;
};

class StartTimerEvent: public IEvent
{
public:
    StartTimerEvent(const TimerPtr &timer, int timeoutMs, int repeatMs)
            : IEvent((int)ELoopError::StartTimer), _timer(timer), _timeoutMs(timeoutMs), _repeatMs(repeatMs) {}

    TimerPtr _timer;
    int _timeoutMs;
    int _repeatMs;
    
};

class StopTimerEvent: public IEvent
{
public:
    StopTimerEvent(const TimerPtr &timer): IEvent((int)ELoopError::StopTimer), _timer(timer) {}

    TimerPtr _timer;
};

} } // namespace ez::uv

#endif // __EZ_UV_LOOP_HPP__
