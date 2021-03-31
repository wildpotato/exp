#ifndef __EZ_PROFILING_TIMER_HPP__
#define __EZ_PROFILING_TIMER_HPP__

#include <chrono>

namespace ez {

class ProfilingTimer {
public:
    
    typedef std::chrono::high_resolution_clock hr_clock;
    typedef hr_clock::time_point tp;

    typedef std::chrono::microseconds us;
    typedef std::chrono::nanoseconds ns;
    
    void Start()
    {
        _lastStart = hr_clock::now();
        _lastCheckpoint = _lastStart;
    }

    // get the time diff between two checkpoint
    template <typename ResolutionType = ns>
    ResolutionType Checkpoint()
    {
        tp now(hr_clock::now());
        ResolutionType diff = std::chrono::duration_cast<ResolutionType>(now - _lastCheckpoint);

        _lastCheckpoint = now;
        return diff;
    }

    // get time diff between start/stop
    template <typename ResolutionType = ns>
    ResolutionType Stop()
    {
        return std::chrono::duration_cast<ResolutionType>(hr_clock::now() - _lastStart);
    }

private:
    
    hr_clock::time_point _lastStart;
    hr_clock::time_point _lastCheckpoint;
};

    
} // namespace ez

#endif // __EZ_PROFILING_TIMER_HPP__
