#ifndef __RAMCACHEDB__H__
#define __RAMCACHEDB__H__
#include <map>
#include <ThreadPool.h>
#include <string>
#include <DynamicVar.h>
#include <set>
#include <StorageInterface.h>
#include <thread>
#include <dependencyInjectionManager.h>
#include <Confs.h>
#include <logger.h>
#include <mutex>

using namespace std;

#define DUMP_FILE_NAME "RamCacheDb.dump.txt"



class RamCacheDBItem {
public:
    DynamicVar value;
    map<string, RamCacheDBItem> childs;
};

class RamCacheDB: public StorageInterface{
private:
    RamCacheDBItem root;

public:
    RamCacheDB(DependencyInjectionManager* dim);
    ~RamCacheDB();
    Confs *confs;
    NLogger *log;

    int dumpIntervalMs = 60*1000;

    string dumpToString(RamCacheDBItem &current);
    void dump();
    void load();

    bool continueRunning = true;
    thread *dumpThread = nullptr;

    string dataDir="";
    bool pendingChanges=false;

    mutex dblocker;

    RamCacheDBItem* _scrollTree(string name, RamCacheDBItem &curr, bool readOnly);

public: 
    /* StorageInterface interface */
    void set(string name, DynamicVar v) override;
    DynamicVar get(string name, DynamicVar defaultValue) override;

    //return only imediate childs, do not return subschilds (childs of childs). Return only imediate key name (the full key name should not be returned)
    vector<string> getChilds(string parentName) override;
    bool hasValue(string name) override;
    void deleteValue(string name, bool deleteChildsInACascade = false) override;
    void forEachChilds(string parentName, function<void(string, DynamicVar)> f) override;
    future<void> forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel) override;

public:
    string getDumpFilePath();
};

#endif
