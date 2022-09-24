#include  "Controller_ClientHelper.test.h" 

Controller_ClientHelperTester::Controller_ClientHelperTester() 
{ 
    
} 

Controller_ClientHelperTester::~Controller_ClientHelperTester() 
{ 
    delete cli;
    delete api;
    delete db;
} 
 
vector<string> Controller_ClientHelperTester::getContexts()
{
    return {"Controller.ClientHelper"};
}

void Controller_ClientHelperTester::run(string context)
{
    if (context != "Controller.ClientHelper") return;

    db = new TmpDBInMemory();
    api = new ApiInterfaceTmp();

    //Controller_ClientHelper(StorageInterface *db, string clientId, ApiInterface* api);

    instantiationTest();
    test_function_getClientId();
    test_function_notify();
    test_function_updateLiveTime_and_getLastLiveTime();
    test_function_timeSinceLastLiveTime();
    test_function_isConnected();
    test_function_registerNewObservation();
    test_function_getObservingVars();
    test_function_unregisterObservation();
    test_function_removeClientFromObservationSystem();
}

void Controller_ClientHelperTester::instantiationTest()
{
    cli = new Controller_ClientHelper(db, clientId, api);

    this->test("Database structure - CientId", [&](){
        auto result = db->get("internal.clients.byId."+clientId, "--no found--");
        return TestResult{
            result.getInt() == 0, 
            "0",
            result.getString()
        };
    });

    this->test("Database structure - ApiId", [&](){
        auto result = db->get("internal.clients.byId."+clientId+".apiId", "--no found--");
        return TestResult{
            result.getString() == api->getApiId(),
            api->getApiId(),
            result.getString()
        };
    });

    this->test("Database structure - List count", [&](){
        auto result = db->get("internal.clients.list.count", 0);
        return TestResult{
            result.getInt() == 1,
            "1",
            result.getString()
        };
    });

    this->test("Database structure - Id on list", [&](){
        auto result = db->get("internal.clients.list.0", "--not found--");
        return TestResult{
            result.getString() == cli->getClientId(),
            cli->getClientId(),
            result.getString()
        };
    });
}

void Controller_ClientHelperTester::test_function_getClientId()
{
    //string getClientId();
    this->test("ClientHelper::getClientId sould return the correct client id", [&](){
        auto result = cli->getClientId();
        
        return TestResult{
            result == cli->getClientId(),
            cli->getClientId(),
            result
        };
    });
}

void Controller_ClientHelperTester::test_function_notify()
{
    //API::ClientSendResult notify(vector<tuple<string, DynamicVar>> varsAndValues);
    string receivedClientIdByApi;
    vector<tuple<string, DynamicVar>> receivedVarsAndValuesByApi;
    ClientSendResult nextApiResult = ClientSendResult::LIVE;


    api->setFunction_notifyClient([&](string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
    {
        receivedClientIdByApi = clientId;
        receivedVarsAndValuesByApi = varsAndValues;
        //return an error (no data exchanged with the client);
        return nextApiResult;
    });

    this->test("ClientHelper::notify sould send the client id to the API", [&](){
        receivedClientIdByApi ="";
        cli->notify(
            { std::make_tuple<string, DynamicVar>("varname", string("varValue")) }
        );

        
        return TestResult{
            receivedClientIdByApi == cli->getClientId(),
            cli->getClientId(),
            receivedClientIdByApi
        };
    });


    this->test("ClientHelper::notify sould send 'varname=varValue' to the API", [&](){
        receivedVarsAndValuesByApi = {};
        cli->notify(
            { std::make_tuple<string, DynamicVar>("varname", string("varValue")) }
        );

        string expected = "varname=varValue";
        string receivedByApi = "";
        for (auto &c: receivedVarsAndValuesByApi)
            receivedByApi = std::get<0>(c) + "=" + std::get<1>(c).getString() + ";";

        if (receivedByApi.size() > -0 && receivedByApi[receivedByApi.size() -1] == ';')
            receivedByApi = receivedByApi.substr(0, receivedByApi.size()-1);
    

        return TestResult{
            receivedByApi == expected,
            expected,
            receivedByApi
        };
    });
}

void Controller_ClientHelperTester::test_function_updateLiveTime_and_getLastLiveTime()
{
    //int64_t getLastLiveTime();

    this->test("If updateLastLiveTime is not called, lastLiveTime should not change", [&](){
        auto t1 = cli->getLastLiveTime();
        usleep(100000);
        auto t2 = cli->getLastLiveTime();

        return TestResult{
            t1 == t2,
            "equal values",
            t1 == t2 ? "equal values" : "different values"
        };
    });

    this->test("If updateLastLiveTime is called, lastLiveTime should be changed", [&](){
        auto t1 = cli->getLastLiveTime();
        usleep(1000000);
        cli->updateLiveTime();
        auto t2 = cli->getLastLiveTime();

        return TestResult{
            t1 != t2,
            "different values",
            t1 == t2 ? "equal values" : "different values"
        };
    });

    this->test("If client not send or receive any data, lastLiveTime should not change", [&](){
        auto t1 = cli->getLastLiveTime();
        usleep(100000);

        //when clientHelper try to notify the client, the api will return an error
        api->setFunction_notifyClient([](string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
        {
            //return an error (no data exchanged with the client);
            return ClientSendResult::DISCONNECTED;
        });

        cli->notify(
            { std::make_tuple<string, DynamicVar>("varname", string("varValue")) }
        );

        auto t2 = cli->getLastLiveTime();

        return TestResult{
            t1 == t2,
            "equal values",
            t1 == t2 ? "equal values" : "different values"
        };
    });

    this->test("If any data is exchanged , lastLiveTime should not change", [&](){
        auto t1 = cli->getLastLiveTime();
        usleep(1000000);

        //when clientHelper try to notify the client, the api will return a sucess result, and the last live time should be updated
        api->setFunction_notifyClient([](string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
        {
            //return an error (no data exchanged with the client);
            return ClientSendResult::LIVE;
        });

        cli->notify(
            { std::make_tuple<string, DynamicVar>("varname", string("varValue")) }
        );
        auto t2 = cli->getLastLiveTime();

        return TestResult{
            t1 != t2,
            "different values",
            t1 == t2 ? "equal values" : "different values"
        };
    });
}

void Controller_ClientHelperTester::test_function_timeSinceLastLiveTime()
{
    //int64_t timeSinceLastLiveTime();

    this->test("If client is not active, timeSinceLastLiveTime should increase", [&]()
    {
        auto t1 = cli->timeSinceLastLiveTime();
        usleep(1000000);
        auto t2 = cli->timeSinceLastLiveTime();

         return t2 > t1;
    });

    this->test("if client is active, timeSinceLastLiveTime counter should be restarted", [&]()
    {

        api->setFunction_notifyClient([](string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
        {
            return ClientSendResult::LIVE;
        });

        auto t1 = cli->timeSinceLastLiveTime();
        usleep(1000000);
        cli->notify(
            { std::make_tuple<string, DynamicVar>("varname", string("varValue")) }
        );
        auto t2 = cli->timeSinceLastLiveTime();

        return t2 < t1;
    });
}

void Controller_ClientHelperTester::test_function_isConnected()
{
    //bool isConnected();
    this->test("isConnected should result false if api return connection error", [&]()
    {

        api->setFunction_checkAlive([](string clientId)
        {
            return ClientSendResult::DISCONNECTED;
        });

        auto result = cli->isConnected();

        return result == false;
    });

    this->test("isConnected should result true if api return success", [&]()
    {

        api->setFunction_checkAlive([](string clientId)
        {
            return ClientSendResult::LIVE;
        });

        auto result = cli->isConnected();

        return result == true;
    });
}

void Controller_ClientHelperTester::test_function_registerNewObservation()
{
    //void registerNewObservation(string varName);
    this->test("registerNewObservation must reset liveTime", [&]()
    {
        auto t1 = db->get("internal.clients.byId."+clientId+".lastLiveTime", 0).getInt64();
        usleep(1000000);
        cli->registerNewObservation("mustreset.livetime");
        auto t2 = db->get("internal.clients.byId."+clientId+".lastLiveTime", 0).getInt64();

        return t2 > t1;
    });

    this->test("registerNewObservation should increase internal list count", [&]()
    {
        auto c1 = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
        cli->registerNewObservation("mustincreate.internalcounter");
        auto c2 = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();

        return c2 > c1;
    });

    this->test("registerNewObservation should create internal list item", [&]()
    {
        string varName = "mustincreate.internallist.item";
        auto itemIndex = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
        cli->registerNewObservation(varName);
        auto registeredItem = db->get("internal.clients.byId."+clientId+".observing."+to_string(itemIndex), "").getString();

        //remove the '.vars', that is added by varHelper to the var name
        if (registeredItem.find("vars.") == 0)
            registeredItem = registeredItem.substr(5);

        return TestResult{
            registeredItem == varName,
            varName, 
            registeredItem
        };
    });
}

void Controller_ClientHelperTester::test_function_getObservingVars()
{
    //vector<string> getObservingVars();
    this->test("ClientHelper::getObservingVars sould return the correct var list", [&](){
        cli->registerNewObservation("another.observation.var");
        string expected = "";
        string received = "";
        auto totalCountOnDb = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
        for (auto c = 0; c < totalCountOnDb; c++)
        {
            auto tmpName = db->get("internal.clients.byId."+clientId+".observing."+to_string(c), "").getString();
            //remove the "vars." from the start of the tmpName
            tmpName = tmpName.substr(5);
            expected += tmpName + " ";
        }

        for (auto &c: cli->getObservingVars())
            received += c + " ";
        
        
        return TestResult{
            received == expected,
            expected,
            received
        };
    });
}

void Controller_ClientHelperTester::test_function_unregisterObservation()
{
    //void unregisterObservation(string varName);
    cli->registerNewObservation("an.observer");
    cli->registerNewObservation("an.anotherObserver");

    auto originalCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();

    cli->unregisterObservation("an.observer");
    
    auto counterAfterRemoving = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();

    auto isOnTheList = [](string find, vector<string> list)
    {
        for (auto &c: list)
            if (c == find)
                return true;
        return false;
    };

    this->test("unregisterObservation should decrease internal counter", [&]()
    {
        return TestResult{
            counterAfterRemoving < originalCount,
            string("< ") + to_string(originalCount),
            to_string(counterAfterRemoving)
        };
    });

    this->test("unregisterObservation should remove a previus added observer", [&]()
    {
        return isOnTheList("an.observer", cli->getObservingVars()) == false;
    });

    this->test("unregisterObservation should not remove another vars", [&]()
    {
        return isOnTheList("an.anotherObserver", cli->getObservingVars()) == true;
    });

    cli->unregisterObservation("an.observer");
    auto counterAfterSecondRemoving = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
    this->test("unregisterObservation should do nothing if deleting a previus deleted var", [&]()
    {
        return TestResult{
            counterAfterSecondRemoving == counterAfterRemoving,
            to_string(counterAfterRemoving),
            to_string(counterAfterSecondRemoving)
        };
    });

    cli->unregisterObservation("an.observer");
    auto counterAfterRemovingInvalidVar = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
    this->test("unregisterObservation should do nothing if deleting a not existing var", [&]()
    {
        return TestResult{
            counterAfterRemovingInvalidVar == counterAfterRemoving,
            to_string(counterAfterRemoving),
            to_string(counterAfterRemovingInvalidVar)
        };
    });
}

void Controller_ClientHelperTester::test_function_removeClientFromObservationSystem()
{
    //void removeClientFromObservationSystem();
    auto originalClientCount = db->get("internal.clients.list.count", 0).getInt();
    cli->removeClientFromObservationSystem();


    this->test("ClientHelper::removeClientFromObservationSystem sould remove internal counter", [&](){
        return db->hasValue("internal.clients.byId."+clientId+".observing.count") == false;
    });

    this->test("ClientHelper::removeClientFromObservationSystem sould remove variable lists", [&](){
        auto variables = db->getChilds("internal.clients.byId."+clientId+".observing");
        string variableNames = "";
        for (auto &c: variables)
            variableNames += c;

        return TestResult{
            variableNames == "",
            "",
            variableNames
        };
    });

    this->test("ClientHelper::removeClientFromObservationSystem should decrease client list size", [&](){
        auto expected = originalClientCount-1;
        auto received = db->get("internal.clients.list.count", 0).getInt();
        return TestResult{
            received == expected,
            to_string(expected),
            to_string(received)
        };
    });

    this->test("ClientHelper::removeClientFromObservationSystem should remove client from client byId list", [&](){
        auto result = db->get("internal.clients.byId."+clientId, "not found").getString();
        return TestResult{
            result == "not found",
            "not found",
            result
        };
    });

    this->test("ClientHelper::removeClientFromObservationSystem should remove client from client list", [&](){
        auto childList = db->getChilds("internal.clients.list");
        bool foundInTheList = false;

        for (auto &c: childList)
        {
            if (db->get("internal.clients.list."+c, "--notFound--").getString() == clientId)
            {
                foundInTheList = true;
                break;
            }
        }

        return foundInTheList == false;
    });
}
