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


    this->testV("Should return default value if conf not exists", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);

        auto retValue=db->get("test", "not found").getString();
        assertEquals("not found", retValue);

        delete db;
    });

    this->testV("Should return correct value", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);
        db->set("test", "test");

        auto retValue=db->get("test", "not found").getString();
        delete db;
        assertEquals("test", retValue);

        return true;
    });

    this->testV("Should dump and load values between instances", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);
        db->set("test2", "test2value");
        delete db;

        RamCacheDB *db2 = new RamCacheDB(&dim);
        auto retValue=db2->get("test2", "not found").getString();
        delete db;

        assertEquals("test2value", retValue);
    });

    this->testV("Should return imediate childs (not child of childs)", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);
        db->set("parent.child1", "--");
        db->set("parent.child2", "--");
        db->set("parent.child3", "--");
        db->set("parent.child4", "--");
        db->set("parent.child5", "--");
        db->set("parent.child1.child1_1", "--");
        db->set("parent.child1.child1_2", "--");
        db->set("parent.child1.child1_3", "--");
        db->set("parent.child1.child1_4", "--");
        db->set("parent.child1.child1_5", "--");

        auto childs = db->getChilds("parent");
        assertEquals(5, childs.size());

        for (auto &c: childs)
            assertNotContains(c, ".", "Childs should not have dots");
    });

    this->testV("Should delete items and childs", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);
        db->set("parent.child1", "--");
        db->set("parent.child2", "--");
        db->set("parent.child3", "--");
        db->set("parent.child4", "--");
        db->set("parent.child5", "--");
        db->set("parent.child1.child1_1", "--");
        db->set("parent.child1.child1_2", "--");
        db->set("parent.child1.child1_3", "--");
        db->set("parent.child1.child1_4", "--");
        db->set("parent.child1.child1_5", "--");

        db->deleteValue("parent.child1", true);

        auto childs = db->getChilds("parent");
        assertEquals(4, childs.size());

        for (auto &c: childs)
            assertNotContains(c, ".", "Childs should not have dots");

        return true;
    });

    this->testV("Should not delete childs if cascade is false", [&](){
        RamCacheDB *db = new RamCacheDB(&dim);
        db->set("parent.child1", "--");
        db->set("parent.child2", "--");
        db->set("parent.child3", "--");
        db->set("parent.child4", "--");
        db->set("parent.child5", "--");
        db->set("parent.child1.child1_1", "--");
        db->set("parent.child1.child1_2", "--");
        db->set("parent.child1.child1_3", "--");
        db->set("parent.child1.child1_4", "--");
        db->set("parent.child1.child1_5", "--");

        db->deleteValue("parent.child1", false);

        auto childs = db->getChilds("parent");
        assertEquals(5, childs.size());

        for (auto &c: childs)
            assertNotContains(c, ".", "Childs should not have dots");

        childs = db->getChilds("parent.child1");
        assertEquals(5, childs.size());

        for (auto &c: childs)
            assertNotContains(c, ".", "Childs should not have dots");

        return true;
    });
}
