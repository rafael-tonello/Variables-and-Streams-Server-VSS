#ifndef VSTP_TEST_H
#define VSTP_TEST_H

#include <string>
#include <regex>
#include <tester.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../../../../sources/Services/APIs/VSTP/VSTP.h"
#include "../../../../../sources/Services/APIs/ApiMediatorInterface.h"
#include "../../../../../sources/Shared/Libs/ThreadPool/ThreadPool.h"
#include <logger.h>
#include <LoggerConsoleWriter.h>

using namespace API;
class  VstpTester: public Tester, public API::ApiMediatorInterface{
private:
    ThreadPool th;
    VSTP *ph;
    int clientSocket = -1;


    future<int> connectToVSTP();
    void testeWriteFunction();
    void testTCPEndPoint();
    void testeProcessPackFunction();

    string convertStringToByteList(string s, size_t i = 0);
public:
    VstpTester(){
        ph = new VSTP(5100, this, new Logger({new LoggerConsoleWriter(LOGGER_LOGLEVEL_DEBUG)}));
    };
    vector<string> getContexts();
    void run(string context);

public:
    /*ApiMediatorInterface*/
    void apiStarted(ApiInterface *api);
    string clientConnected(string clientId, ApiInterface* api);
    void observeVar(string varName, string clientId, ApiInterface* api);
    void stopObservingVar(string varName, string clientId, ApiInterface* api);
    future<void> lockVar(string varName);
    future<void> unlockVar(string varName);
    
    future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue);
    future<void> setVar(string name, DynamicVar value);
    future<void> delVar(string varname);
    future<vector<string>> getChildsOfVar(string parentName);

        
        
        
        
        

};

        


#endif