#include "uv/Timer.hpp"
#include <cassert>

namespace ez { namespace uv {


ReturnCode Timer::_Start(uv_loop_t *loop, int timeoutMs, int repeatMs)
{
    assert(_uvtimer == nullptr);
    assert(timeoutMs >= 0);
    assert(repeatMs >= 0);
    
    _uvtimer = new uv_timer_t;
    
    int status = uv_timer_init(loop, _uvtimer);
    if (status != 0)
    {
        delete _uvtimer;
        _uvtimer = nullptr;
        return {-1, status};
    }

    uv_handle_set_data((uv_handle_t *)_uvtimer, this);

    status = uv_timer_start(_uvtimer,
                            [] (uv_timer_t* handle)
                            {
                                uv_loop_t* loop = uv_handle_get_loop((uv_handle_t *)handle);
                                Timer *timer = (Timer *)uv_handle_get_data((uv_handle_t *)handle);

                                if (timer->IsEnable())
                                    timer->_onTimeoutCB(timer, uv_now(loop));
                            },
                            timeoutMs, repeatMs);

    if (status == 0)
    {
        return {};
    }
    else
    {
        delete _uvtimer;
        _uvtimer = nullptr;
        return {-2, status};
    }
}

ReturnCode Timer::_Stop()
{
    if (_uvtimer == nullptr)
        return {};

    int status = uv_timer_stop(_uvtimer);
    if (status != 0)
        return {-1, status};

    uv_close((uv_handle_t*)_uvtimer, [] (uv_handle_t *handle) { delete handle; });
    _uvtimer = nullptr;

    return {};   
}

} } // namespace ez::uv