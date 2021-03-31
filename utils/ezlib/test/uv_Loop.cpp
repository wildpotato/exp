#include "catch.hpp"
#include <chrono>
#include <thread>
#include "uv/Loop.hpp"

using namespace ez;
using namespace ez::uv;

TEST_CASE("loop start/stop", "[uv][Loop]")
{
    bool eventReceived(false);
    bool hasError(false);
    
    Loop loop;
    ReturnCode rc = loop.Start(
            [&eventReceived] (IEventPtr &eventPtr)
            {
                eventReceived = true;
                REQUIRE(eventPtr->GetType() == 1);
            },
            [&hasError] (ELoopError err, const std::string &msg) { hasError = true; } );
            
    REQUIRE(rc.GetCode() == 0);

    rc = loop.SendEvent(CreateEvent<IEvent>(1));
    REQUIRE(rc.GetCode() == 0);

    // waiting event to be handled
    using namespace std::chrono_literals;
    while (eventReceived == false)
    {
        std::this_thread::sleep_for(5ms);
    }

    loop.Stop();

    REQUIRE(hasError == false);
}

TEST_CASE("loop's dtor should properly stop loop", "[uv][Loop]")
{
    {
        bool eventReceived(false);

        Loop loop;
        ReturnCode rc = loop.Start(
                [&eventReceived] (IEventPtr &eventPtr)
                {
                    eventReceived = true;
                    REQUIRE(eventPtr->GetType() == 1);
                },
                [] (ELoopError err, const std::string &msg) { });

        REQUIRE(rc.GetCode() == 0);

        rc = loop.SendEvent(CreateEvent<IEvent>(1));
        REQUIRE(rc.GetCode() == 0);

        // waiting event to be handled
        using namespace std::chrono_literals;
        while (eventReceived == false)
        {
            std::this_thread::sleep_for(5ms);
        }
    }

    SUCCEED();
}

TEST_CASE("loop's dtor should doing fine when loop not started", "[uv][Loop]")
{
    {
        Loop loop;
    }

    SUCCEED();
}

TEST_CASE("loop's timer start/stop", "[uv][Loop][Timer]")
{
    bool eventReceived(false);
    bool hasError(false);

    int counter(0);
    int type(1);
    TimerPtr timer = Timer::Create(type, [&counter] (Timer *timer, uint64_t loopTime) 
                                        {
                                            ++counter;
                                        });

    Loop loop;
    ReturnCode rc = loop.Start(
            [&eventReceived] (IEventPtr &eventPtr) { eventReceived = true; },
            [&hasError] (ELoopError err, const std::string &msg) { hasError = true; } );

    REQUIRE(rc.GetCode() == 0);

    rc = loop.StartTimer(timer, 5, 5);
    REQUIRE(rc.GetCode() == 0);

    using namespace std::chrono_literals;
    while (counter <= 2)
    {
        std::this_thread::sleep_for(5ms);
    }
    
    loop.StopTimer(timer);
    
    std::this_thread::sleep_for(5ms);

    loop.Stop();

    REQUIRE(eventReceived == false);
    REQUIRE(hasError == false);
}

TEST_CASE("loop stop cleanly without stop/close timer first", "[uv][Loop][Timer]")
{
    bool eventReceived(false);
    bool hasError(false);

    int counter(0);
    int type(1);
    TimerPtr timer = Timer::Create(type, [&counter] (Timer *timer, uint64_t loopTime) 
                                        {
                                            ++counter;
                                        });

    Loop loop;
    ReturnCode rc = loop.Start(
            [&eventReceived] (IEventPtr &eventPtr) { eventReceived = true; },
            [&hasError] (ELoopError err, const std::string &msg) { hasError = true; } );

    REQUIRE(rc.GetCode() == 0);

    rc = loop.StartTimer(timer, 5, 5);
    REQUIRE(rc.GetCode() == 0);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
    
    REQUIRE(counter > 0);

    loop.Stop();

    REQUIRE(eventReceived == false);
    REQUIRE(hasError == false);
}

TEST_CASE("loop's timer not repeating", "[uv][Loop][Timer]")
{
    bool eventReceived(false);
    bool hasError(false);

    int counter(0);
    int type(1);
    TimerPtr timer = Timer::Create(type, [&counter] (Timer *timer, uint64_t loopTime) 
                                        {
                                            ++counter;
                                        });

    Loop loop;
    ReturnCode rc = loop.Start(
            [&eventReceived] (IEventPtr &eventPtr) { eventReceived = true; },
            [&hasError] (ELoopError err, const std::string &msg) { hasError = true; } );

    REQUIRE(rc.GetCode() == 0);

    rc = loop.StartTimer(timer, 5, 0);
    REQUIRE(rc.GetCode() == 0);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15ms);

    REQUIRE(counter == 1);
    
    loop.Stop();

    REQUIRE(eventReceived == false);
    REQUIRE(hasError == false);
}
