#include  "Controller_VarHelper.test.h" 
 
Controller_VarHelperTester::Controller_VarHelperTester() 
{ 
     
} 
 
Controller_VarHelperTester::~Controller_VarHelperTester() 
{ 
     
} 
 
vector<string> Controller_VarHelperTester::getContexts()
{
    return { "Controller.VarHelper" };
}

void Controller_VarHelperTester::run(string context)
{
    if (context != "Controller.VarHelper") return;

    instantiationTest();
    test_function_setFlag();
    test_function_getFlag();
    test_function_getValue();
    test_function_setValue();
    test_function_isLocked();
    test_function_valueIsSetInTheDB();
    test_function_lock();
    test_function_unlock();
    test_function_deleteFromDB();
    test_function_getChildsNames();
    test_function_isClientObserving();
    test_function_addClientToObservers();
    test_function_removeClientFromObservers();
    test_function_foreachObserversClients();
    test_function_getObserversClientIds();
}

void Controller_VarHelperTester::instantiationTest()
{

}

void Controller_VarHelperTester::test_function_setFlag()
{
    //void setFlag(string flagName, DynamicVar value);
}

void Controller_VarHelperTester::test_function_getFlag()
{
    //DynamicVar getFlag(string flagName, DynamicVar defaultValue = "");
}

void Controller_VarHelperTester::test_function_getValue()
{
    //DynamicVar getValue(DynamicVar defaultValue = "");
}

void Controller_VarHelperTester::test_function_setValue()
{
    //void setValue(DynamicVar value);
}

void Controller_VarHelperTester::test_function_isLocked()
{
    //bool isLocked();
}

void Controller_VarHelperTester::test_function_valueIsSetInTheDB()
{
    //bool valueIsSetInTheDB();
}

void Controller_VarHelperTester::test_function_lock()
{
    //void lock();
}

void Controller_VarHelperTester::test_function_unlock()
{
    //void unlock();
}

void Controller_VarHelperTester::test_function_deleteFromDB()
{
    //void deleteFromDB();
}

void Controller_VarHelperTester::test_function_getChildsNames()
{
    //vector<string> getChildsNames();
}

void Controller_VarHelperTester::test_function_isClientObserving()
{
    //bool isClientObserving(string clientId);
}

void Controller_VarHelperTester::test_function_addClientToObservers()
{
    //void addClientToObservers(string clientId);
}

void Controller_VarHelperTester::test_function_removeClientFromObservers()
{
    //void removeClientFromObservers(string clientId);
}

void Controller_VarHelperTester::test_function_foreachObserversClients()
{
    //void foreachObserversClients(FObserversForEachFunction f);
}

void Controller_VarHelperTester::test_function_getObserversClientIds()
{
    //vector<string> getObserversClientIds();
}
