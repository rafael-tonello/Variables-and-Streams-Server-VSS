#ifndef __VARSYSTEMLIBSTORAGE__H__ 
#define __VARSYSTEMLIBSTORAGE__H__ 

#include "../StorageInterface.h"
#include <FileVars.h>
#include <memory>
#include <dependencyInjectionManager.h>
#include <Confs.h>
#include <ThreadPool.h>
 
using namespace std;
using namespace Shared;



class VarSystemLibStorage: public StorageInterface { 
private:
    shared_ptr<FileVars> db;
    Shared::Config *confs;
public: 
    VarSystemLibStorage(DependencyInjectionManager* dim);
    void set(string name, DynamicVar v);
    DynamicVar get(string name, DynamicVar defaultValue);
    vector<string> getChilds(string parentName);
    bool hasValue(string name);
    void deleteValue(string name, bool deleteChildsInACascade = false);
    void forEach(string parentName, function<void(string, DynamicVar)> f);
    future<void> forEach_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel);
}; 
 
#endif 
