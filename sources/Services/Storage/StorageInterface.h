#ifndef __STORAGEINTERFACE__H__ 
#define __STORAGEINTERFACE__H__ 

#include <string>
#include <vector>
#include <DynamicVar.h>
#include <ThreadPool.h>

using namespace std;
 
class StorageInterface { 
public: 
    virtual void set(string name, DynamicVar v) = 0;
    virtual DynamicVar get(string name, DynamicVar defaultValue) = 0;
    virtual vector<string> getChilds(string parentName) = 0;
    virtual bool hasValue(string name) = 0;
    virtual void deleteValue(string name, bool deleteChildsInACascade = false) = 0;
    virtual ~StorageInterface(){}
    virtual void forEachChilds(string parentName, function<void(string, DynamicVar)> f) = 0;
    virtual future<void> forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel) = 0;

}; 
 
#endif 
