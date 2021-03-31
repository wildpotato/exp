#ifndef __EZ_LOCK_QUEUE_HPP__
#define __EZ_LOCK_QUEUE_HPP__

#include <mutex>
#include <deque>
#include <condition_variable>
#include <chrono>

namespace ez {

// simple lock queue supporting wait/notify
template <typename T>
class LockQueue
{
public:
    LockQueue() {}

    ~LockQueue() {}

    void Push(const T &data)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        _queue.push_back(data);
    }
    void Push(T &&data)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        _queue.push_back(std::move(data));
    }

    
    bool TryPop(T& data)
    {
        std::lock_guard<std::mutex> guard(_mutex);
        
        if (!_queue.empty())
        {
            data = std::move(_queue.front());
            _queue.pop_front();
            return true;
        }
        else
            return false;
    }
    
    void PushAndNotify(const T &data)
    {
        Push(data);
        _cv.notify_one();
    }
    void PushAndNotify(T &&data)
    {
        Push(std::move(data));
        _cv.notify_one();
    }
    
    bool WaitAndPop(T& data, int ms = 1000)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_cv.wait_for(lock, std::chrono::milliseconds(ms), [this] { return !_queue.empty(); }))
        {
            data = std::move(_queue.front());
            _queue.pop_front();            
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    std::mutex _mutex;
    std::deque<T> _queue;
    std::condition_variable _cv;
};


} // namespace ez

#endif // __EZ_LOCK_QUEUE_HPP__