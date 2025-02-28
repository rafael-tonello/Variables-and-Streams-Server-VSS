#ifndef __RAMCACHEDB__H__
#define __RAMCACHEDB__H__
#include <map>
#include <ThreadPool.h>
#include <string>
#include <DynamicVar.h>
#include <set>
#include <StorageInterface.h>


using namespace std;

class RamCacheDB: public StorageInterface{
private:
    map<string, DynamicVar> db;
public:
    RamCacheDB();
    ~RamCacheDB();

public: 
    /* StorageInterface interface */
    void set(string name, DynamicVar v) override;
    DynamicVar get(string name, DynamicVar defaultValue) override;

    //return only imediate childs, do not return subschilds (childs of childs). Return only imediate key name (the full key name should be returned)
    vector<string> getChilds(string parentName) override;
    bool hasValue(string name) override;
    void deleteValue(string name, bool deleteChildsInACascade = false) override;
    void forEachChilds(string parentName, function<void(string, DynamicVar)> f) override;
    future<void> forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel) override;
};

#endif
