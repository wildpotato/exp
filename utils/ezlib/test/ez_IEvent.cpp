#include "catch.hpp"
#include "IEvent.hpp"

using namespace ez;

class DerivedEvent: public IEvent
{
public:
    DerivedEvent(int type, int data): IEvent(type), _data(data) {}
    int _data;
};

TEST_CASE("IEvent creation", "[ez][IEvent]")
{
    SECTION("create by default ctor")
    {
        IEventPtr event = CreateEvent<IEvent>();
        REQUIRE(event->GetType() == -1);
    }

    SECTION("create with args")
    {
        IEventPtr event = CreateEvent<IEvent>(1);
        REQUIRE(event->GetType() == 1);
    }

    SECTION("create from derived type")
    {
        IEventPtr event = CreateEvent<DerivedEvent>(2, 3);
        REQUIRE(event->GetType() == 2);

        DerivedEvent *realPtr = (DerivedEvent *)event.get();
        REQUIRE(realPtr->_data == 3);
    }
}
