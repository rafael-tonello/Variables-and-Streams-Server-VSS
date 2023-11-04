#ifndef __VARSYSTEMLIBSTORAGE__H__ 
#define __VARSYSTEMLIBSTORAGE__H__ 

#include "../StorageInterface.h"
#include <FileVars.h>
#include <memory>
#include <dependencyInjectionManager.h>
#include <Confs.h>
#include <ThreadPool.h>
#include <utils.h>
#include <logger.h>
 
using namespace std;
using namespace Shared;



class VarSystemLibStorage: public StorageInterface { 
private:
    shared_ptr<FileVars> db;
    Confs *confs;

    string escape(string text);
    string unescape(string text);
    NLogger *log;
    string databaseLocation;
public: 
    VarSystemLibStorage(DependencyInjectionManager* dim);
    void set(string name, DynamicVar v);
    DynamicVar get(string name, DynamicVar defaultValue);
    vector<string> getChilds(string parentName);
    bool hasValue(string name);
    void deleteValue(string name, bool deleteChildsInACascade = false);
    void forEachChilds(string parentName, function<void(string, DynamicVar)> f);
    future<void> forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel);

    string getDatabseFolder();
}; 
 
#endif 
