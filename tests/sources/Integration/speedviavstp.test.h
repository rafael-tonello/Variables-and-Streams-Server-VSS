#ifndef __SPEEDVIAVSTP.TEST__H__
#define __SPEEDVIAVSTP.TEST__H__
#include <tester.h>
#include <logger.h>

struct SpeedViaVstpTestLogInfo
{
    Logger* sender = NULL;
    string msg = "";
    int level = 0;
    string name = "";
};

class SpeedViaVstpTest: public Tester{
private:
    SpeedViaVstpTestLogInfo lastLogInfo;
    TmpDBInMemory database;
    DependencyInjectionManager dim;

    int connectToServer(string server, int port);
public:
    SpeedViaVstpTest();
    ~SpeedViaVstpTest();

    vector<string> getContexts() override;
    void run(string context) override;
};

#endif
