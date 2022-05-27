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

    db = new TmpDBInMemory();
    logger = new Logger({ 
        new LoggerLambdaWriter([](Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel)
        {
            if (aboveOrInLogLevel)
            cout << "\t\t\t\t[" << name << "] " << msg << endl;
        })
    });

    var = new Controller_VarHelper(logger, db, varName);

    test_function_setFlag();
    test_function_getFlag();
    test_function_lock();
    test_function_unlock();
    test_function_isLocked();
    test_function_setValue();
    test_function_getValue();
    test_function_deleteFromDB();
    test_function_getChildsNames();
    test_function_isClientObserving();
    test_function_addClientToObservers();
    test_function_removeClientFromObservers();
    test_function_foreachObserversClients();
    test_function_getObserversClientIds();
}

void Controller_VarHelperTester::test_function_setFlag()
{
     //void setFlag(string flagName, DynamicVar value);

    this->test("Show create internal flag", [&](){
        this->var->setFlag("aFlag", "aValue");

        //check if the variable flag was createad in the database
        return db->get("vars."+varName + "._aFlag", "-- not found --").getString() == "aValue";

    });
 
    this->test("Flags can't came back with getChilds function call", [&]()
    {
        bool flagFound = false;
        auto tmp = db->getChilds("vars."+varName);
        for (auto &c: db->getChilds("vars."+varName))
        {
            if (c.find("_") == 0)
                flagFound = true;
        }

        return flagFound == false;
    });
}

void Controller_VarHelperTester::test_function_getFlag()
{
    //DynamicVar getFlag(string flagName, DynamicVar defaultValue = "");
    this->test("getFlag should return the value 'aValue'", [&](){
        auto result = this->var->getFlag("aFlag", "-- not found --").getString();

        return TestResult{
            result == "aValue",
            "aValue",
            result
        };
    });
}

void Controller_VarHelperTester::test_function_lock()
{
    this->test("lock should create the lock flag in the databse", [&](){
        this->var->lock();
        auto lockValue = db->get("vars."+varName + "._lock", "--not found--").getString();

        return TestResult{
            lockValue == "1",
            "1",
            lockValue
        };
    });

    this->test("lock should wait another locking to return", [&](){
        //NOTE: the variable is already locked by the previus test
        int64_t initialTime = Utils::getCurrentTimeMilliseconds();
        int64_t finalTime = initialTime;
        
        //create a thread to wait for the lock oportunity
        bool threadExitNormally = false;
        thread th([&](){
            this->var->lock();

            finalTime = Utils::getCurrentTimeMilliseconds();
            threadExitNormally = true;

        });
        th.detach();

        //awayt 1 second and unlock the var
        usleep(500000);
        db->set("vars."+varName + "._lock", "0");
        usleep(50000);

        //try kill the thread if lock can't return to normally kill the thread
        if (!threadExitNormally)
            pthread_cancel(th.native_handle());

        auto totalTime = finalTime - initialTime;

        return TestResult{
            totalTime >= 500 && totalTime <= 510,
            ">=500 ms and <=510 ms",
            to_string(totalTime) + " ms"
        };
    });
}

void Controller_VarHelperTester::test_function_unlock()
{   
    //grant that variable is not remain locked by old tests
    db->set("vars."+varName + "._lock", "0");

    //lock the variable
    this->var->lock();

    this->test("unlock should set the lock flag in the databse with the value 0", [&](){
        this->var->unlock();
        auto lockValue = db->get("vars."+varName + "._lock", "1").getString();

        return TestResult{
            lockValue == "0",
            "0",
            lockValue
        };
    });

    this->test("unlock should do nothing to a unlocked var", [&](){
        this->var->unlock();
        auto lockValue = db->get("vars."+varName + "._lock", "1").getString();

        return TestResult{
            lockValue == "0",
            "0",
            lockValue
        };
    });
}

void Controller_VarHelperTester::test_function_isLocked()
{
    //grant that variable is not remain locked by old tests
    db->set("vars."+varName + "._lock", "0");

    this->test("isLocked should return 'true' if variable is locked", [&](){
        //lock variable directly in the databse (to remove relation with possible errors in the "lock" and "unlock" functions)
        db->set("vars."+varName + "._lock", "1");
        auto lockValue = this->var->isLocked();

        return TestResult{
            lockValue == true,
            "true",
            lockValue ? "true" : "false"
        };
    });

    this->test("isLocked should return 'false' if variable is not locked", [&](){
        //unlock variable directly in the databse (to remove relation with possible errors in the "lock" and "unlock" functions)
        db->set("vars."+varName + "._lock", "0");
        auto lockValue = this->var->isLocked();

        return TestResult{
            lockValue == false,
            "false",
            lockValue ? "true" : "false"
        };
    });

}

void Controller_VarHelperTester::test_function_setValue()
{
    //void setValue(DynamicVar value);

    this->test("setValue method should create correct database key and value", [&](){
        this->var->setValue("test value");
        auto result = db->get("vars."+varName, "--not found--").getString();

        return TestResult{
            result == "test value",
            "test value",
            result
        };
    });


    this->test("setValue can't work if the var name contains a wildcard ('*')", [&](){
        string varName2 = "a.wildcard.var.*";
        auto var2 = new Controller_VarHelper(logger, db, varName2);

        var2->setValue("test value");

        auto result = db->get("vars."+varName2, "--not found--").getString();

        delete var2;

        return TestResult{
            result == "--not found--",
            "--not found--",
            result
        };
    });
    
}

void Controller_VarHelperTester::test_function_getValue()
{
    //DynamicVar getValue(DynamicVar defaultValue = "");

    this->test("getValue should return the correct value ('test value' string)", [&](){
        auto result = this->var->getValue("--not found--").getString();

        return TestResult{
            result == "test value",
            "test value",
            result
        };
    });

    this->test("getValue should return nothing if variable not exist in the db", [&](){
        string varName2 = "another.variable";
        auto var2 = new Controller_VarHelper(logger, db, varName2);

        auto result = var2->getValue("--not found--").getString();

        delete var2;

        return TestResult{
            result == "--not found--",
            "--not found--",
            result
        };
    });
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
