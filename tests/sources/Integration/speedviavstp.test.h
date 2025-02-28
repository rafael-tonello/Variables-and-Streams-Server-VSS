#ifndef __SPEEDVIAVSTP_TEST__H__
#define __SPEEDVIAVSTP_TEST__H__ 
#include <tester.h>
#include <logger.h>
#include <TmpDBInMemory.h>
#include <dependencyInjectionManager.h>
#include <LoggerLambdaWriter.h>
#include <IConfProvider.h>
#include <InMemoryConfProvider.h>
#include <Confs.h>

#include <sys/socket.h>
#include <arpa/inet.h>

struct SpeedViaVstpTestLogInfo
{
    ILogger* sender = NULL;
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
