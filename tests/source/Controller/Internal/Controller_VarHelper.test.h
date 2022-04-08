#ifndef __CONTROLLER_VARHELPERTEST__H__ 
#define __CONTROLLER_VARHELPERTEST__H__ 

#include <tester.h>
 
class Controller_VarHelperTester: public Tester { 
private:
    void instantiationTest();
    void test_function_setFlag();
    void test_function_getFlag();
    void test_function_getValue();
    void test_function_setValue();
    void test_function_isLocked();
    void test_function_valueIsSetInTheDB();
    void test_function_lock();
    void test_function_unlock();
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
