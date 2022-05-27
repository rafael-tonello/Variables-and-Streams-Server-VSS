#ifndef _CONTROLLER_TEST_H_
#define _CONTROLLER_TEST_H_

#include <tester.h>
#include "../../../sources/Controller/Controller.h"
#include <map>
#include <logger.h>
#include <writers/LoggerConsoleWriter.h>
#include <mocks/TmpDBInMemory.h>
#include "../mocks/InMemoryConfProvider.h"

using namespace std;
using namespace Controller;
class ControllerTester: public Tester{
private:
    DependencyInjectionManager dim;
    TheController *ctrl;

    void test_function__createUniqueId();
    void test_function_updateClientAboutObservatingVars();
    void test_function_notifyClient();
    void test_function_deleteClient();
    void test_function_checkClientLiveTime();
    void test_function_internalSetVar();
    void test_function_getVarInternalFlag();
    void test_function_setVarInternalFlag();
    void test_function_notifyVarModification();
    void test_function_notifyParentGenericObservers();
    void test_function_notifyClientsAboutVarChange();
    void test_function_apiStarted();
    void test_function_clientConnected();
    void test_function_observeVar();
    void test_function_stopObservingVar();
    void test_function_getVar();
    void test_function_setVar();
    void test_function_delVar();
    void test_function_getChildsOfVar();
    void test_function_lockVar();
    void test_function_unlockVar();


    shared_ptr<IConfigurationProvider> getConfigurationProvider();

    
    
public:

    ControllerTester(); 
public:
    /*Tester 'interface'*/
    vector<string> getContexts();
    void run(string context);
};

#endif