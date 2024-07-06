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

    //return only imediate childs, do not return subschilds (childs of childs). Return only imediate key name (the full key name should be returned)
    virtual vector<string> getChilds(string parentName) = 0;
    virtual bool hasValue(string name) = 0;
    virtual void deleteValue(string name, bool deleteChildsInACascade = false) = 0;
    virtual ~StorageInterface(){}
    
    virtual void forEachChilds(string parentName, function<void(string, DynamicVar)> f){
        auto childs = getChilds(parentName);
        for (auto &curr: childs)
            f(curr, this->get(curr, ""));
    }
    virtual future<void> forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel){
        auto names = this->getChilds(parentName);
        vector<future<void>> *pendingTasks = new vector<future<void>>;
        for (auto &c: names)
        {
            pendingTasks->push_back(taskerForParallel->enqueue([&](string name){
                f(name, this->get(name, ""));
            }, c));
        }

        auto ret = taskerForParallel->enqueue([&, pendingTasks](){
            for (auto &c: *pendingTasks)
                c.wait();

            pendingTasks->clear();
            delete pendingTasks;
        });

        return ret;
    }
}; 
 
#endif 
