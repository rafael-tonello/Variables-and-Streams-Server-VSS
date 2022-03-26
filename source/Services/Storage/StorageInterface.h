#ifndef __STORAGEINTERFACE__H__ 
#define __STORAGEINTERFACE__H__ 

#include <string>
#include <vector>
#include <DynamicVar.h>
#include <ThreadPool.h>

using namespace std;
using namespace Shared;
 
class StorageInterface { 
public: 
    virtual bool set(string name, DynamicVar v) = 0;
    virtual DynamicVar get(string name, DynamicVar defaultValue) = 0;
    virtual vector<string> getChilds(string parentName) = 0;
    virtual bool existsValue(string name) = 0;
    virtual bool deleteValue(string name, bool deleteChildsInACascade = false) = 0;
    virtual ~StorageInterface(){}
    virtual void forEach(string parentName, function<void(DynamicVar)> f) = 0;
    virtual future<void> forEach_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel) = 0;

}; 
 
#endif 
