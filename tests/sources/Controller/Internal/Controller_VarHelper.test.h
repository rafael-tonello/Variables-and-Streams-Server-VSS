#ifndef __CONTROLLER_VARHELPERTEST__H__ 
#define __CONTROLLER_VARHELPERTEST__H__ 

#include <tester.h>
#include <StorageInterface.h>
#include <Controller_VarHelper.h>
#include <logger.h>
#include <LoggerLambdaWriter.h>
#include <mocks/TmpDBInMemory.h>
 
class Controller_VarHelperTester: public Tester { 
private:
    TmpDBInMemory *db;
    ILogger *logger;

    Controller_VarHelper *var;
    string varName = "the.var.name";

    void test_function_setFlag();
    void test_function_getFlag();
    void test_function_lock();
    void test_function_unlock();
    void test_function_isLocked();
    void test_function_setValue();
    void test_function_getValue();
    void test_function_deleteFromDB();
    void test_function_getChildsNames();
    void test_function_isClientObserving();
    void test_function_addClientToObservers();
    void test_function_removeClientFromObservers();
    void test_function_foreachObserversClients();
    void test_function_getObserversClientIds();

public: 
    Controller_VarHelperTester(); 
    ~Controller_VarHelperTester(); 
public:
    /* Tester class */
    vector<string> getContexts();
    void run(string context);
}; 
 
#endif 
