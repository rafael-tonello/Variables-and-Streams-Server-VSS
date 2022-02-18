#ifndef PHOMAU_TEST_H
#define PHOMAU_TEST_H

#include <string>
#include <regex>
#include <tester.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../../../../source/Services/APIs/PHOMAU/PHOMAU.h"
#include "../../../../../source/Services/APIs/ApiMediatorInterface.h"
#include "../../../../../source/Shared/Libs/ThreadPool/ThreadPool.h"
#include <logger.h>
#include <logger/writers/LoggerConsoleWriter.h>

using namespace API;
class PhomauTester: public Tester, public API::ApiMediatorInterface{
private:
    ThreadPool th;
    PHOMAU *ph;
    int clientSocket = -1;


    future<int> connectToPHOMAU();
    void testeWriteFunction();
    void testTCPEndPoint();
    void testeProcessPackFunction();

    string convertStringToByteList(string s, size_t i = 0);
public:
    PhomauTester(){
        ph = new PHOMAU(5100, this, new Logger({new LoggerConsoleWriter(true)}, Logger::LOGGER_DEBUG_LEVEL));
    };
    vector<string> getContexts();
    void run(string context);

public:
    /*ApiMediatorInterface*/
    void apiStarted(ApiInterface *api);
    string clientConnected(string clientId, ApiInterface* api);
    void observeVar(string varName, string clientId, ApiInterface* api);
    void stopObservingVar(string clientId, string varName);
    future<void> lockVar(string varName);
    future<void> unlockVar(string varName);
    
    future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue);
    future<void> setVar(string name, DynamicVar value);
    future<void> delVar(string varname);
    future<vector<string>> getChildsOfVar(string parentName);

        
        
        
        
        

};

        


#endif