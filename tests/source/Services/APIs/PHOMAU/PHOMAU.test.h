#ifndef PHOMAU_TEST_H
#define PHOMAU_TEST_H

#include <string>
#include <regex>
#include <tester.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../../../../source/Services/APIs/PHOMAU/PHOMAU.h"
#include "../../../../../source/Services/APIs/ApiMediatorInterface.h"
#include "../../../../../source/Shared/ThreadPool/ThreadPool.h"

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
        ph = new PHOMAU(5100, this);
    };
    vector<string> getContexts();
    void run(string context);

public:
    /*ApiMediatorInterface*/
    future<void> createAlias(string name, string dest);
    future<string> getAliasValue(string aliasName);
    future<void> deleteAlias(string aliasName);

    string observeVar(string varName, observerCallback callback, void* args = NULL, string observerId = "");
    void stopObservingVar(string observerId);
    future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue);
    
    future<void> setVar(string name, DynamicVar value);
    future<void> delVar(string varname);
    future<vector<string>> getChildsOfVar(string parentName);

};

        


#endif