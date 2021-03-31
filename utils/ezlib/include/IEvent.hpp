#ifndef __EZ_IEVENT_HPP__
#define __EZ_IEVENT_HPP__

#include <memory>
#include <type_traits>

namespace ez {

class IEvent
{
public:
    // user should use type >= 0
    IEvent(int type = -1): _type(type) {}
    virtual ~IEvent() {}

    int GetType() const { return _type; }

private:
    int _type;
};

using IEventPtr = std::shared_ptr<IEvent>;

template <typename EventType, typename ...Args> IEventPtr CreateEvent(Args && ...args)
{
    static_assert(std::is_convertible<EventType*, IEvent*>::value, "EventType must inherit IEvent");

    auto newEventPtr = std::make_shared<EventType>(std::forward<Args>(args)...);
    return std::dynamic_pointer_cast<IEvent>(newEventPtr);
}

} // namespace ez

#endif // __EZ_IEVENT_HPP__
