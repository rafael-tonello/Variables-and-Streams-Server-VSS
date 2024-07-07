#include "Controller.test.h"

ControllerTester::ControllerTester()
{

    auto logger = new Logger({ 
        new LoggerLambdaWriter([&](Logger* sender, string msg, int level, string name, std::time_t dateTime)
        {
            this->lastLogInfo = {sender, msg, level, name};
            cout << "\t\t\t\t[" << name << "] " << msg << endl;
        })
    });

    dim.addSingleton<ILogger>(logger);
    dim.addSingleton<ThreadPool>(new ThreadPool(4));
    dim.addSingleton<Confs>(new Confs({getConfigurationProvider()}), {typeid(Confs).name()});
    dim.addSingleton<StorageInterface>(&database);
}


vector<string> ControllerTester::getContexts()
{
    return {"Controller"};
}

void ControllerTester::run(string context)
{
    if (context == "Controller")
    {
        this->ctrl = new TheController(&this->dim, "test-version");
        test_function__createUniqueId();
        test_function_setVar();
        test_function_getVar();
        test_function_delVar();
        test_function_observeVar();
        test_function_stopObservingVar();
        test_function_updateClientAboutObservatingVars();
        test_function_notifyClient();
        test_function_deleteClient();
        test_function_checkClientLiveTime();
        test_function_internalSetVar();
        test_function_getVarInternalFlag();
        test_function_setVarInternalFlag();
        test_function_notifyVarModification();
        test_function_notifyParentGenericObservers();
        test_function_notifyClientsAboutVarChange();
        test_function_apiStarted();
        test_function_clientConnected();
        test_function_getChildsOfVar();
        test_function_lockVar();
        test_function_unlockVar();       
    }
}

void ControllerTester::test_function__createUniqueId()
{
    this->test("_createUniqueId() sould return different ids between calls", [&](){
        auto id1 = ctrl->_createUniqueId();
        auto id2 = ctrl->_createUniqueId();

        return id1 != id2;
    });
}

void ControllerTester::test_function_setVar()
{
    this->test("setVar should not be able to create flags", [&](){
        this->ctrl->setVar("sampleVar._flag", "the flag value").get();

        auto result = dim.get<StorageInterface>()->get("vars.sampleVar._flag", "--not found--").getString();
        return TestResult{
            result == "--not found--", 
            "--not found--", 
            result
        };
    });

    this->test("setVar should return erro when try to create flags", [&](){
        auto result = this->ctrl->setVar("sampleVar._flag", "the flag value").get();

        return TestResult{
            result != Errors::NoError, 
            "any error message ( != \"\")", 
            result
        };
    });

    this->test("setVar should not be able to create vars with '*' as a child name", [&](){
        this->ctrl->setVar("sampleVar.*", "the var value").get();

        auto result = dim.get<StorageInterface>()->get("vars.sampleVar.*", "--not found--").getString();
        return TestResult{
            result == "--not found--", 
            "--not found--", 
            result
        };
    });

    this->test("setVar should return error when try to set '*' as a child name", [&](){
        auto result = this->ctrl->setVar("sampleVar.*", "the var value").get();

        return TestResult{
            result != Errors::NoError, 
            "any error message ( != \"\")", 
            result
        };
    });
    
    
    
    this->test("setVar should not be able to create vars with '*' as part of name", [&](){
        this->ctrl->setVar("sampleVar.var*name", "the var value").get();

        auto result = dim.get<StorageInterface>()->get("vars.sampleVar.var*name", "--not found--").getString();
        return TestResult{
            result == "--not found--", 
            "--not found--", 
            result
        };
    });
    

    this->test("setVar should not be able to the var name '*'", [&](){
        this->ctrl->setVar("*", "the var value").get();

        auto result = dim.get<StorageInterface>()->get("vars.*", "--not found--").getString();
        return TestResult{
            result == "--not found--", 
            "--not found--", 
            result
        };
    });

    this->test("setVar should return error when try to set '*'", [&](){
        auto result = this->ctrl->setVar("*", "the var value").get();

        return TestResult{
            result != Errors::NoError, 
            "any error message ( != \"\")", 
            result
        };
    });

    

    this->test("setVar should return error when try to set '*'", [&](){
        auto result = this->ctrl->setVar("sampleVar/var*name", "the var value").get();

        return TestResult{
            result != Errors::NoError, 
            "any error message ( != \"\")", 
            result
        };
    });





    this->test("setVar should create variable in the database", [&](){
        this->ctrl->setVar("sampleVar", "the variable value").get();

        auto storedValue = dim.get<StorageInterface>()->get("vars.sampleVar", "--not found--").getString();
        return TestResult{
            storedValue == "the variable value", 
            "the variable value", 
            storedValue
        };
    });

    this->test("setVar should create variable of a var child in the database", [&](){
        this->ctrl->setVar("this.is.a.var.with.childs", "the child value").get();

        auto storedValue = dim.get<StorageInterface>()->get("vars.this.is.a.var.with.childs", "--not found--").getString();
        return TestResult{
            storedValue == "the child value", 
            "the child value", 
            storedValue
        };
    });

    //test setting a flag
    this->test("setVar can't set flag (vars begining with '_')", [&](){
        this->ctrl->setVar("_sampleVar", "value").get();

        auto storedValue = dim.get<StorageInterface>()->get("vars._sampleVar", "--not found--").getString();
        return TestResult{
            storedValue == "--not found--", 
            "--not found--", 
            storedValue
        };
    });

    //testing setting a locked var
    this->test("setVar can't set locked (vars begining with '_')", [&](){
        this->ctrl->setVar("sampleVar._flag", "value").get();

        auto storedValue = dim.get<StorageInterface>()->get("vars.sampleVar._flag", "--not found--").getString();
        return TestResult{
            storedValue == "--not found--", 
            "--not found--", 
            storedValue
        };
    });

    this->test("setValue cannot set a locked var", [&](){
        string varName = "this_var_will.be.locked";
        string originalVarValue = "this is the original value of the locked var";
        this->ctrl->setVar(varName, originalVarValue).get();

        //lock variable directly in the database to avoid relation with lockVar and unlockVar methods
        dim.get<StorageInterface>()->set("vars."+varName+"._lock", 1);

        this->ctrl->setVar(varName, "the value of a the locked vas was changed").get();

        string valueInTheDb = dim.get<StorageInterface>()->get("vars."+varName, "--not found--").getString();

        return TestResult{
            valueInTheDb == originalVarValue,
            originalVarValue,
            valueInTheDb
        };
    });
}

void ControllerTester::test_function_getVar()
{
    this->test("getVar should result in error for empty names", [&](){
        auto retValue = this->ctrl->getVar("", "default value").get();

        return TestResult{
            retValue.status != Errors::NoError, 
            "Any error message", 
            retValue.status
        };
    });

    this->test("getVar should default value for variables that not exist", [&](){
        auto returnedValue = std::get<1>(this->ctrl->getVar("this.var.not.exists.in.the.database", "default value").get().result[0]).getString();

        return TestResult{
            returnedValue == "default value", 
            "default value", 
            returnedValue
        };
    });

    this->test("getVar should return correct value of a existing variable", [&](){
        this->ctrl->setVar("this.variable.exists", "exisisting value").wait();
        auto returnedValue = std::get<1>(this->ctrl->getVar("this.variable.exists", "default value").get().result[0]).getString();

        return TestResult{
            returnedValue == "exisisting value",
            "exisisting value",
            returnedValue
        };
    });

    this->test("getVar should return a list of vars if wildcard char (*) is used", [&](){
        string v1name = "variables.var1";
        string v2name = "variables.var2";
        string v3name = "variables.var2.achild";
        string v4name = "variables.var2.anotherchild";
        string v5name = "variables.var3.var3child";

        this->ctrl->setVar(v1name, "exisisting value").wait();
        this->ctrl->setVar(v2name, "exisisting value").wait();
        this->ctrl->setVar(v3name, "exisisting value").wait();
        this->ctrl->setVar(v4name, "exisisting value").wait();
        this->ctrl->setVar(v5name, "exisisting value").wait();



        auto variables = this->ctrl->getVar("variables.*", "default value").get().result;
        auto varWasReturned = [&](string vName){
            for (auto &c: variables)
                if (std::get<0>(c) == vName)
                    return true;
            return false;
        };

        bool v1Found = varWasReturned(v1name);
        bool v2Found = varWasReturned(v2name);
        bool v3Found = varWasReturned(v3name);
        bool v4Found = varWasReturned(v4name);
        bool v5Found = varWasReturned(v5name);

        return v1Found && v2Found && v2Found && v3Found && v4Found && v5Found;
    });

    this->test("getVar should return a list of vars for 'variabl*' string", [&](){
        string v1name = "variables.var1";
        string v2name = "variables.var2";
        string v3name = "variables.var2.achild";
        string v4name = "variables.var2.anotherchild";
        string v5name = "variables.var3.var3child";


        auto variables = this->ctrl->getVar("variables.*", "default value").get().result;
        auto varWasReturned = [&](string vName){
            for (auto &c: variables)
                if (std::get<0>(c) == vName)
                    return true;
            return false;
        };

        bool v1Found = varWasReturned(v1name);
        bool v2Found = varWasReturned(v2name);
        bool v3Found = varWasReturned(v3name);
        bool v4Found = varWasReturned(v4name);
        bool v5Found = varWasReturned(v5name);

        return v1Found && v2Found && v2Found && v3Found && v4Found && v5Found;
    });

    this->test("getVar should return a list of vars if only the '*' char is used", [&](){
        auto variables = this->ctrl->getVar("*", "default value").get().result;
        return variables.size() > 5;
    });

}

void ControllerTester::test_function_delVar()
{
    this->test("delVar should delete variables", [&](){
        string v1name = "variables.var1";
        string v2name = "variables.var2";
        string v3name = "variables.var2.achild";
        string v4name = "variables.var2.anotherchild";
        string v5name = "variables.var3.var3child";

        this->ctrl->setVar(v1name, "exisisting value").wait();
        this->ctrl->setVar(v2name, "exisisting value").wait();
        this->ctrl->setVar(v3name, "exisisting value").wait();
        this->ctrl->setVar(v4name, "exisisting value").wait();
        this->ctrl->setVar(v5name, "exisisting value").wait();

        this->ctrl->delVar(v1name).wait();
        this->ctrl->delVar(v2name).wait();
        this->ctrl->delVar(v3name).wait();
        this->ctrl->delVar(v4name).wait();
        this->ctrl->delVar(v5name).wait();


        auto variables = this->ctrl->getVar("variables.*", "default value").get();
        auto varValueExistsOnDb = [&](string vName){
            return dim.get<StorageInterface>()->get("vars."+vName, "--not found--").getString() != "--not found--";
        };

        bool v1NotFound = !varValueExistsOnDb(v1name);
        bool v2NotFound = !varValueExistsOnDb(v2name);
        bool v3NotFound = !varValueExistsOnDb(v3name);
        bool v4NotFound = !varValueExistsOnDb(v4name);
        bool v5NotFound = !varValueExistsOnDb(v5name);

        return v1NotFound && v2NotFound && v2NotFound && v3NotFound && v4NotFound && v5NotFound;
    });

    //delVar should't accept '*' char

}

void ControllerTester::test_function_observeVar()
{
    ApiInterfaceTmp apit;
    string clientId = Utils::createUniqueId();
    string customId = Utils::createUniqueId();
    string varName = "this.is.a.var";
    StorageInterface *db = dim.get<StorageInterface>();

    // runLocked([&](){

    // int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();
    
    // db->set(name + "._observers.list.count", actualVar_observersCount);
    // db->set(name + "._observers.list."+to_string(actualVar_observersCount), clientId);
    // db->set(name + "._observers.byId."+clientId, actualVar_observersCount);
    this->test("observeVar should increment the client count in the variable (vars session of db)", [&](){
        int initialCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();
        this->ctrl->observeVar(varName, clientId, customId, &apit);
        int finalCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();

        return TestResult{
            finalCount == initialCount+1,
            to_string(initialCount+1),
            to_string(finalCount)
        };
    });

    this->test("observeVar should save the client index with the corret value (vars session of db)", [&](){
        int correctValue = db->get("vars."+varName+"._observers.list.count", "0").getInt()-1;
        int savedValue = db->get("vars."+varName+"._observers.byId."+clientId+".byMetadata."+customId, "-1").getInt();

        return TestResult{
            correctValue == savedValue,
            to_string(correctValue),
            to_string(savedValue)
        };
    });

    this->test("observeVar should save the client id int the observers list (vars session of db)", [&](){
        int index = db->get("vars."+varName+"._observers.byId."+clientId+".byMetadata."+customId, "-1").getInt();

        string savedId = db->get("vars."+varName+"._observers.list."+to_string(index)+".clientId", "--not found--").getString();

        return TestResult{
            savedId == clientId,
            clientId,
            savedId
        };
    });



    //     varName = "vars."+varName;
    // Utils::named_lock("db.intenal.clients", [&](){
    //     this->updateLiveTime();

    //     auto currentCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
    //     db->set("internal.clients.byId."+clientId+".observing.count", currentCount+1);
    //     db->set("internal.clients.byId."+clientId+".observing."+to_string(currentCount), varName);


    string varName2 = "this.is.another.var";
    auto itemIndex = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
    auto prevClientObservingCount = itemIndex;
    this->ctrl->observeVar(varName2, clientId, customId, &apit);

    this->test("observeVar should increase internal list items count (clients session of db)", [&]()
    {
        auto currentClientObservindCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();        

        return TestResult{
            currentClientObservindCount == prevClientObservingCount+1,
            to_string(prevClientObservingCount + 1), 
            to_string(currentClientObservindCount)
        };
    });

    this->test("observeVar should create internal list item (clients session of db)", [&]()
    {
        auto registeredItem = db->get("internal.clients.byId."+clientId+".observing."+to_string(itemIndex), "").getString();

        return TestResult{
            registeredItem == "vars."+varName2,
            varName2, 
            registeredItem
        };
    });

}

void ControllerTester::test_function_stopObservingVar()
{
    ApiInterfaceTmp apit;
    string clientId = Utils::createUniqueId();
    string customId = Utils::createUniqueId();
    string clientId2 = Utils::createUniqueId();
    string customId2 = Utils::createUniqueId();
    string varName = "this.is.a.var";
    StorageInterface *db = dim.get<StorageInterface>();
    this->ctrl->observeVar(varName, clientId, customId, &apit);
    int theClientIndex = db->get("vars."+varName+"._observers.list.count", "0").getInt();


    this->ctrl->observeVar(varName, clientId2, customId2, &apit);

    

    this->test("observeVar should save client id in the client list (vars session of db)", [&](){
        auto expectedId = clientId2;
        auto receivedId = db->get("vars."+varName+"._observers.list."+to_string(theClientIndex) + ".clientId", "--not found--").getString();

        return TestResult{
            receivedId == expectedId,
            expectedId,
            receivedId
        };
    });

    this->test("stopObservingVar should decrement the client count in the variable (vars session of db)", [&](){
        int initialCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();;
        this->ctrl->stopObservingVar(varName, clientId, customId, &apit);
        int finalCount = db->get("vars."+varName+"._observers.list.count", "0").getInt();

        return TestResult{
            finalCount == initialCount-1,
            to_string(initialCount-1),
            to_string(finalCount)
        };
    });


    string varName2 = "this.is.another.var";
    string additionalId = "abc";
    this->ctrl->observeVar(varName2, clientId, additionalId, &apit);
    auto itemIndex = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();

    this->test("stopObservingVar should increase internal list items count (clients session of db)", [&]()
    {
        auto prevClientObservingCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
        this->ctrl->stopObservingVar(varName2, clientId, additionalId, &apit);
        auto currentClientObservindCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();

        return TestResult{
            currentClientObservindCount == prevClientObservingCount-1,
            to_string(prevClientObservingCount - 1), 
            to_string(currentClientObservindCount)
        };
    });

    this->test("observeVar should remove var from internal list item (clients session of db)", [&]()
    {
        auto result = db->get("internal.clients.byId."+clientId+".observing."+to_string(itemIndex), "--not found--").getString();

        return TestResult{
            result == "--not found--",
            "--not found--", 
            result
        };
    });

}

void ControllerTester::test_function_updateClientAboutObservatingVars()
{

}

void ControllerTester::test_function_notifyClient()
{

}

void ControllerTester::test_function_deleteClient()
{

}

void ControllerTester::test_function_checkClientLiveTime()
{

}

void ControllerTester::test_function_internalSetVar()
{

}

void ControllerTester::test_function_getVarInternalFlag()
{

}

void ControllerTester::test_function_setVarInternalFlag()
{

}

void ControllerTester::test_function_notifyVarModification()
{

}

void ControllerTester::test_function_notifyParentGenericObservers()
{

}

void ControllerTester::test_function_notifyClientsAboutVarChange()
{

}

void ControllerTester::test_function_apiStarted()
{

}

void ControllerTester::test_function_clientConnected()
{

}

void ControllerTester::test_function_getChildsOfVar()
{

}

void ControllerTester::test_function_lockVar()
{

}

void ControllerTester::test_function_unlockVar()
{

}

IConfProvider* ControllerTester::getConfigurationProvider()
{
    IConfProvider *confProvider = new InMemoryConfProvider(
    {
        std::make_tuple("sample", "value")
    });

    return confProvider;
}