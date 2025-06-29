#ifndef __CONTROLLER_CLIENTHELPERTEST__H__ 
#define __CONTROLLER_CLIENTHELPERTEST__H__ 

#include <tester.h>
#include <StorageInterface.h>
#include <Controller_ClientHelper.h>
#include <mocks/ApiInterfaceTmp.h>
#include <mocks/TmpDBInMemory.h>

class Controller_ClientHelperTester: public Tester { 
private:
    TmpDBInMemory *db;
    ApiInterfaceTmp *api;

    Controller_ClientHelper *cli;
    string clientId = "clientTestId";

    void test_function_getClientId();
    void test_function_notify();
    void test_function_updateLiveTime_and_getLastLiveTime();
    void test_function_timeSinceLastLiveTime();
    void test_function_isConnected();
    void test_function_registerNewObservation();
    void test_function_getObservingVars();
    void test_function_unregisterObservation();
    void test_function_removeClientFromObservationSystem();

    void instantiationTest();
public: 
    Controller_ClientHelperTester();
    ~Controller_ClientHelperTester();
public:
    /* Tester class */
    vector<string> getContexts();
    void run(string context);
};


 
#endif 
