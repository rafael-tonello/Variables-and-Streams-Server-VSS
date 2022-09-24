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
        new LoggerLambdaWriter([](Logger* sender, string msg, int level, string name)
        {
            
            cout << "\t\t\t\t[" << name << "] " << msg << endl;
        })
    });

    var = new Controller_VarHelper(logger, db, varName);

    this->yellowMessage("setFlag and getFlag");
    test_function_setFlag();
    test_function_getFlag();

    this->yellowMessage("lock, unlock and isLocked");
    test_function_lock();
    test_function_unlock();
    test_function_isLocked();

    this->yellowMessage("setValue, getValue and deleteValueFromDB");
    test_function_setValue();
    test_function_getValue();
    test_function_deleteValueFromDB();

    this->yellowMessage("getChildsNames");
    test_function_getChildsNames();

    this->yellowMessage("addClientsToObservers, removeClientFromObservers, isClientObserving, foreachObserversClients and getObserversClientIds");
    test_function_addClientToObservers();
    test_function_removeClientFromObservers();
    test_function_isClientObserving();
    test_function_getObserversClientIds();
    test_function_foreachObserversClients();
}

void Controller_VarHelperTester::test_function_setFlag()
{
     //void setFlag(string flagName, DynamicVar value);

    this->test("Show create internal flag", [&](){
        this->var->setFlag("aFlag", "aValue");

        //check if the variable flag was createad in the database
        return db->get("vars."+varName + "._aFlag", "-- not found --").getString() == "aValue";

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

        //awayt 500 milisseconds and unlock the var
        usleep(500000);
        db->set("vars."+varName + "._lock", "0");
        usleep(50000);

        //try kill the thread if lock can't return to normally kill the thread
        if (!threadExitNormally)
            pthread_cancel(th.native_handle());

        auto totalTime = finalTime - initialTime;

        return TestResult{
            totalTime >= 500 && totalTime <= 515,
            ">=500 ms and <=515 ms (bet 500 and 510 ±5)",
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

void Controller_VarHelperTester::test_function_deleteValueFromDB()
{
    //void deleteValueFromDB();
    this->var->setFlag("thisFlagMustBeKept", "the flag is in the db");
    this->var->deleteValueFromDB();

    this->test("deleteValueFromDB should delete only the value in the database", [&](){
        auto result = this->db->get("vars."+varName, "--not found--").getString();

        return TestResult{
            result == "--not found--",
            "--not found--",
            result
        };
    });

    this->test("deleteValueFromDB should keep flags in the database", [&](){
        auto result = this->db->get("vars."+varName+"._thisFlagMustBeKept", "--not found--").getString();

        return TestResult{
            result == "the flag is in the db",
            "the flag is in the db",
            result
        };
    });
}

void Controller_VarHelperTester::test_function_getChildsNames()
{
    //vector<string> getChildsNames();
    Controller_VarHelper child1_normal(logger, db, varName + ".child1");
    Controller_VarHelper child2_only_with_flags(logger, db, varName + ".child2");
    Controller_VarHelper child3_only_with_another_sub_childs(logger, db, varName + ".child3.child31");

    child1_normal.setValue("the value of child 1");
    child2_only_with_flags.setFlag("chlild2flag", "the child2flag value");
    child3_only_with_another_sub_childs.setValue("a value");
    var->setFlag("getChildsNamesTestFlag", "flagvalue");


    this->test("getChildsNames can't return flags", [&]()
    {
        bool flagFound = false;
        for (auto &c: var->getChildsNames())
        {
            if (c.find("_") == 0)
                flagFound = true;
        }

        return flagFound == false;
    });

    this->test("getChildsNames should return vars with value ", [&]()
    {
        bool flagFound = false;
        auto childs = var->getChildsNames();

        auto count = std::count(childs.begin(), childs.end(), "child1");

        return count == 1;
    });

    this->test("getChildsNames should return vars without value (just with flags)", [&]()
    {
        bool flagFound = false;
        auto childs = var->getChildsNames();

        auto count = std::count(childs.begin(), childs.end(), "child2");

        return count == 1;
    });

    this->test("getChildsNames should return empty vars with another childs", [&]()
    {
        bool flagFound = false;
        auto childs = var->getChildsNames();

        auto count = std::count(childs.begin(), childs.end(), "child3");

        return count == 1;
    });


}

void Controller_VarHelperTester::test_function_addClientToObservers()
{
    //void addClientToObservers(string clientId);
    auto clientId = Utils::createUniqueId();

    this->test("addClientToObservers should increment the client count in the variable", [&](){
        int initialCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();
        var->addClientToObservers(clientId);
        int finalCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();

        return TestResult{
            finalCount == initialCount+1,
            to_string(initialCount+1),
            to_string(finalCount)
        };
    });

    this->test("addClientToObservers should save the client index with the corret value", [&](){
        int correctValue = db->get("vars."+varName+"._observers.list.count", "0").getInt()-1;
        int savedValue = db->get("vars."+varName+"._observers.byId."+clientId, "-1").getInt();

        return TestResult{
            correctValue == savedValue,
            to_string(correctValue),
            to_string(savedValue)
        };
    });

    this->test("addClientToObservers should save the client id int the observers list", [&](){
        int index = db->get("vars."+varName+"._observers.byId."+clientId, "-1").getInt();

        string savedId = db->get("vars."+varName+"._observers.list."+to_string(index), "--not found--").getString();

        return TestResult{
            savedId == clientId,
            clientId,
            savedId
        };
    });
}

void Controller_VarHelperTester::test_function_removeClientFromObservers()
{
    //void removeClientFromObservers(string clientId);
    auto clientId1 = Utils::createUniqueId();
    auto clientId2 = Utils::createUniqueId();
    auto clientId3 = Utils::createUniqueId();
    auto clientId4 = Utils::createUniqueId();
    auto clientId5 = Utils::createUniqueId();
    auto clientId6 = Utils::createUniqueId();
    auto clientId7 = Utils::createUniqueId();
    auto clientId8 = Utils::createUniqueId();
    auto clientId9 = Utils::createUniqueId();
    auto clientId10 = Utils::createUniqueId();

    var->addClientToObservers(clientId1);
    var->addClientToObservers(clientId2);
    var->addClientToObservers(clientId3);
    var->addClientToObservers(clientId4);
    var->addClientToObservers(clientId5);
    var->addClientToObservers(clientId6);
    var->addClientToObservers(clientId7);
    var->addClientToObservers(clientId8);
    var->addClientToObservers(clientId9);
    var->addClientToObservers(clientId10);
    

    this->test("removeClientFromObservers should decrese the client count in the var flags", [&](){
        auto initialCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();
        this->var->removeClientFromObservers(clientId8);
        auto finalCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();

        return TestResult{
            finalCount < initialCount,
            to_string(initialCount-1),
            to_string(finalCount)
        };
    });

    this->test("removeClientFromObservers should shift the client list in the var flags", [&](){
        auto initialValue = db->get("vars."+varName+"._observers.4", "--not found").getString();
        this->var->removeClientFromObservers(clientId5);
        auto finalValue = db->get("vars."+varName+"._observers.4", "--not found--").getString();

        return TestResult{
            initialValue != finalValue,
            "!= "+initialValue,
            finalValue
        };
    });

    this->test("removeClientFromObservers should remove info in the byId list", [&](){
        //auto initialValue = db->get("vars."+varName+"._observers.byId"+clientId2, "--not found--").getString();
        this->var->removeClientFromObservers(clientId2);
        auto finalValue = db->get("vars."+varName+"._observers.byId"+clientId2, "--not found--").getString();

        return TestResult{
            finalValue == "--not found--",
            "--not found--",
            finalValue
        };
    });
}

void Controller_VarHelperTester::test_function_isClientObserving()
{
    auto clientId1 = Utils::createUniqueId();

    var->addClientToObservers(clientId1);

    this->test("isClientObserving should return false for clients that isn't observing a var", [&](){
        auto observingResult = var->isClientObserving("an_id_of_a_client_that_is_not_observing_the_var");
        return observingResult == false;
    });
    

    this->test("isClientObserving should return true for clients observing a var", [&](){
        auto observingResult = var->isClientObserving(clientId1);
        return observingResult;
    });

    this->test("isClientObserving should return false for clients removed from var observing flags", [&](){
        var->removeClientFromObservers(clientId1);
        auto observingResult = var->isClientObserving("an_id_of_a_client_that_is_not_observing_the_var");
        return observingResult == false;
    });
    
}

void Controller_VarHelperTester::test_function_getObserversClientIds()
{
    //vector<string> getObserversClientIds();
    vector<string> clientIds;
    for (auto c = 0; c < 10; c++)
    {
        auto tmpClientId = Utils::createUniqueId();
        var->addClientToObservers(tmpClientId);
        clientIds.push_back(tmpClientId);
    }

    this->test("getObservingClientIds should return all added clients", [&](){
        auto returnedClientIds = var->getObserversClientIds();

        bool allClientsFound = true;
        for (auto &c: clientIds)
        {
            allClientsFound &= std::count(returnedClientIds.begin(), returnedClientIds.end(), c) > 0;
        }

        return allClientsFound;
    });

    this->test("getObservingClientIds should not return id of a removed client", [&](){
        auto idToRemove = clientIds[2];
        var->removeClientFromObservers(idToRemove);

        auto returnedClientIds = var->getObserversClientIds();
        bool removeClienWasFound = false;
        for (auto &c: returnedClientIds)
        {
            if (c == idToRemove)
                removeClienWasFound = true;
        }

        return removeClienWasFound == false;
    });

}

void Controller_VarHelperTester::test_function_foreachObserversClients()
{
    //void foreachObserversClients(FObserversForEachFunction f);
    vector<string> clientIds;
    for (auto c = 0; c < 10; c++)
    {
        auto tmpClientId = Utils::createUniqueId();
        var->addClientToObservers(tmpClientId);
        clientIds.push_back(tmpClientId);
    }

    this->test("getObservingClientIds should return all added clients", [&](){
        string allClientIds = "";
        var->foreachObserversClients([&](string currId){
            allClientIds += currId;
        });

        bool allClientsFound = true;
        for (auto &c: clientIds)
        {
            allClientsFound &= allClientIds.find(c) != string::npos;
        }

        return allClientsFound;
    });

    this->test("getObservingClientIds should not return id of a removed client", [&](){
        auto idToRemove = clientIds[2];
        var->removeClientFromObservers(idToRemove);

        string allClientIds = "";
        var->foreachObserversClients([&](string currId){
            allClientIds += currId;
        });

        return allClientIds.find(idToRemove) == string::npos;
    });


}
