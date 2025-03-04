#include "ramcachedb.test.h"

RamCacheDBTest::RamCacheDBTest()
{

}
  

RamCacheDBTest::~RamCacheDBTest()
{
    
}

vector<string> RamCacheDBTest::getContexts()
{
    return {"Storage.RamCacheDB"};

}

void RamCacheDBTest::run(string context)
{
    if ( context != "Storage.RamCacheDB" )
        return;

    Utils::ssystem("rm -rf /tmp/_vss_ramcachedb_tests_database");

    DependencyInjectionManager dim;

    auto logger = new Logger({ 
        new LoggerLambdaWriter([&](ILogger* sender, string msg, int level, string name, std::time_t dateTime)
        {
            cout << "\t\t\t\t[" << name << "] " << msg << endl;
        })
    });
    dim.addSingleton<ILogger>(logger);

    dim.addSingleton<Confs>(new Confs());
    dim.get<Confs>()->createAlias("DbDirectory").setDefaultValue("/tmp/_vss_ramcachedb_tests_database");
    dim.get<Confs>()->createAlias("RamCacheDbDumpIntervalMs").setDefaultValue("100");


    this->test("Should return default value if conf not exists", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);

        auto retValue=db->get("test", "not found").getString();
        assertEquals("not found", retValue);

        delete db;

        return true;
    });

    this->test("Should return correct value", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);
        db->set("test", "test");

        auto retValue=db->get("test", "not found").getString();
        delete db;
        assertEquals("test", retValue);

        return true;
    });

    this->test("Should dump and load values between instances", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);
        db->set("test2", "test2value");
        delete db;

        RamCacheDB *db2 = new RamCacheDB(&dim);
        auto retValue=db2->get("test2", "not found").getString();
        delete db;

        assertEquals("test2value", retValue);

        return true; 
    });
}
