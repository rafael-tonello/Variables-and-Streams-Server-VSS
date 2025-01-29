#ifndef __PREFIXTHREESTORAGE__H__
#define __PREFIXTHREESTORAGE__H__
#include <StorageInterface.h>
#include <DynamicVar.h>
#include <string>
#include <map>
#include <mutex>
#include <dependencyInjectionManager.h>
#include <ThreadPool.h>
#include <prefixtree.h>
#include <prefixtreestorage/storages/filestorage.h>
#include <Confs.h>

using namespace std;

class PrefixThreeStorage: public StorageInterface{
private:
    mutex m;
    bool useMemoryCache = true;
    uint maxCacheSize = 0;
    uint currentCacheSize = 0;

    map<string, DynamicVar> cache;
    ThreadPool *tasks = nullptr;

    FileStorage *fileStroage=nullptr;
    PrefixTree<DynamicVar> *tree = nullptr;


    void init(DependencyInjectionManager *dep);
public: /* StorageInterface interface */
    void set(string name, DynamicVar v) override;
    DynamicVar get(string name, DynamicVar defaultValue) override;

    //return only imediate childs, do not return subschilds (childs of childs). Return only imediate key name (the full key name should be returned)
    vector<string> getChilds(string parentName) override;
    bool hasValue(string name) override;
    void deleteValue(string name, bool deleteChildsInACascade = false) override;
    
public:
    PrefixThreeStorage(DependencyInjectionManager *dep);
    PrefixThreeStorage(DependencyInjectionManager *dep, bool useMemoryCache, size_t maxCacheSize = 0);
    ~PrefixThreeStorage();
};

#endif
