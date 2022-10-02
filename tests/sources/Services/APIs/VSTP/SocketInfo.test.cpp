#include "SocketInfo.test.h"

vector<string> SocketInfoTester::getContexts()
{
    return {"APIs.VSTP"};

}

void SocketInfoTester::run(string context)
{
    if (context == "APIs.VSTP")
    {
        this->yellowMessage("SocketInfo testes");
        long lastId = 0;
        this->test("SocketInfo::getId() at first run should return a random number", [&](){
            lastId = this->socketInfo.getId();
            return TestResult {
                lastId != 0,
                "!= 0",
                to_string(lastId)
            };
        });

        bool getIdRunsResult = true;
        for (int c = 0; c < 5; c++)
            getIdRunsResult &= this->socketInfo.getId() == lastId;
        this->test("SocketInfo::getId(). Next executions should return allays "+to_string(lastId), [&](){ return getIdRunsResult; });

        ThreadPool th;
        

        this->test("SocketInfo::getAliveSeconds() should 0", [&](){
            this->socketInfo.upadateAlive();
            int time = this->socketInfo.getAliveSeconds();

            return TestResult{
                time == 0,
                "0",
                to_string(time)
            };
        });

        this->test("SocketInfo::getAliveSeconds() should return near 2", [&](){
            usleep(2000000);
            int time = this->socketInfo.getAliveSeconds();
            return TestResult{
                time >=1 && time <= 2,
                "2",
                to_string(time)
            };
            
        });

        this->test("SocketInfo::getAliveSeconds() should return near 5", [&](){
            usleep(1000000);
            int time = this->socketInfo.getAliveSeconds();
            return TestResult{
                time >=2 && time <= 4,
                "3",
                to_string(time)
            };
        });

        this->test("SocketInfo::getAliveSeconds() should return near 3", [&](){
            this->socketInfo.upadateAlive();
            usleep(1000000);
            int time = this->socketInfo.getAliveSeconds();
            return TestResult{
                time >0 && time <= 2,
                "1",
                to_string(time)
            };
        });

        this->test("SocketInfo::getAliveSeconds() should return near 0", [&](){
            this->socketInfo.upadateAlive();
            int time = this->socketInfo.getAliveSeconds();

            return TestResult{
                time == 0,
                "0",
                to_string(time)
            };
        });

    }
}