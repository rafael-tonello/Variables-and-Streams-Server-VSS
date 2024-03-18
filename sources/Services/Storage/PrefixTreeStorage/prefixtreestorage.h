#ifndef __PREFIXTREESTORAGE__H__ 
#define __PREFIXTREESTORAGE__H__ 
#include <StorageInterface.h>
#include <prefixtree.h>
#include <prefixtreestorage/storages/filestorage.h>
#include <memorystorage.h>
#include <dependencyInjectionManager.h>
#include <set>
#include <vector>
#include <Confs.h>
#include <logger.h>


using namespace std;

class PrefixTreeStorage: public StorageInterface {
private:
    IBlockStorage *dbStorage;
    PrefixTree<DynamicVar> *db;

    string dbFile;
    Confs *confs;
    NLogger *log;
public:
    PrefixTreeStorage(DependencyInjectionManager* dim);
    ~PrefixTreeStorage();
    void set(string name, DynamicVar v);
    DynamicVar get(string name, DynamicVar defaultValue);
    //should return complete childs names (for childs of "a.b.c", should return "a.b.c.d, a.b.c.e")
    vector<string> getChilds(string parentName);
    bool hasValue(string name);
    void deleteValue(string name, bool deleteChildsInACascade = false);
};
 
#endif 
