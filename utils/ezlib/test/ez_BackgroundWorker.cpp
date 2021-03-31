#include "catch.hpp"
#include <chrono>
#include <thread>
#include "BackgroundWorker.hpp"

using namespace ez;

class FakeJob: public IJob
{
public:
    FakeJob(): IJob(123) { }
    
    virtual ReturnCode DoIt() { return ReturnCode(1, 2); };
};

TEST_CASE("BackgroundWorker basic usage", "[ez][BackgroundWorker]")
{
    bool jobDone(false);
    
    BackgroundWorker bgWorker;
    ReturnCode rc = bgWorker.Start(
            [&jobDone] (IJobPtr &job, ReturnCode rc)
            {
                REQUIRE(job->GetType() == 123);
                REQUIRE(rc.GetCode() == 1);
                REQUIRE(rc.GetStatus() == 2);
                jobDone = true;
            });
    
    REQUIRE(rc.GetCode() == 0);
            
    IJobPtr job = CreateJob<FakeJob>();
    rc = bgWorker.AddJob(std::move(job));
    
    REQUIRE(rc.GetCode() == 0);
    
    using namespace std::chrono_literals;
    while (jobDone == false)
    {
        std::this_thread::sleep_for(10ms);
    }
    
    bgWorker.Stop();
}