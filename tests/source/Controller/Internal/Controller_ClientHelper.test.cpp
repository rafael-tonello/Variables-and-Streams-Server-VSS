#include  "Controller_ClientHelper.test.h" 



Controller_ClientHelperTester::Controller_ClientHelperTester() 
{ 
     
} 
 
vector<string> Controller_ClientHelperTester::getContexts()
{
    return {"Controller.CLientHelper"};
}

void Controller_ClientHelperTester::run(string context)
{
    if (context != "Controller.CLientHelper") return;

    //Controller_ClientHelper(StorageInterface *db, string clientId, ApiInterface* api);

    TmpDB db;
    ApiInterfaceTmp api;

    Controller_ClientHelper cli(&db, "test", &api);

    this->test("Database structure - CientId", [&](){
        auto result = db.get("internal.clients.byId.test", "--no found--");
        return TestResult{
            result.getInt() == 0, 
            "0",
            result.getString()
        };
    });

    this->test("Database structure - ApiId", [&](){
        auto result = db.get("internal.clients.byId.test.apiId", "--no found--");
        return TestResult{
            result.getString() == api.getApiId(),
            api.getApiId(),
            result.getString()
        };
    });

    this->test("Database structure - List count", [&](){
        auto result = db.get("internal.clients.list.count", 0);
        return TestResult{
            result.getInt() == 0,
            "0",
            result.getString()
        };
    });

    this->test("Database structure - Id on list", [&](){
        auto result = db.get("internal.clients.list.0", "--not found--");
        return TestResult{
            result.getString() == cli.getClientId(),
            cli.getClientId(),
            result.getString()
        };
    });

}
