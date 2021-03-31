#ifndef __EZ_UV_TIMER_HPP__
#define __EZ_UV_TIMER_HPP__

#include <memory>
#include <functional>
#include "uv.h"
#include "ReturnCode.hpp"
#include "IEvent.hpp"

namespace ez { namespace uv {

    
// new timer implement
class Timer;
using TimerPtr = std::shared_ptr<Timer>;

class Timer
{
public:
    typedef std::function<void(Timer *timer, uint64_t loopTime)> OnTimeoutCB;
    
    static TimerPtr Create(int type, OnTimeoutCB callback) { return std::make_shared<Timer>(type, callback); }

    Timer(int type, OnTimeoutCB callback): _type(type), _enable(true), _uvtimer(nullptr), _onTimeoutCB(callback) {}

    ~Timer() {}
    
    // prevent copy/assignment/move, because uv timer is not safe to move while running
    Timer(const Timer &timer) = delete;
    Timer(Timer &&timer) = delete;
    Timer &operator=(const Timer &timer) = delete;
    Timer &operator=(Timer &&timer) = delete;
    
    int GetType() const { return _type; }

    bool IsEnable() const { return _enable; }
    void Disable() { _enable = false; }
    
    // user should not calling these
    ReturnCode _Start(uv_loop_t *loop, int timeoutMs, int repeatMs);
    ReturnCode _Stop();
    
private:
    int _type;
    bool _enable;
    uv_timer_t *_uvtimer;
    OnTimeoutCB _onTimeoutCB;
};


} } // namespace ez::uv

#endif // __EZ_UV_TIMER_HPP__
