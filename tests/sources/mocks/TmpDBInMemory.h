#ifndef __TMP_DB_IN_MEMORY__
#define __TMP_DB_IN_MEMORY__

#include <StorageInterface.h>
#include <future>
#include <string>
#include <map>

using namespace std;

class TmpDBInMemory: public StorageInterface
{
public:
    map<string, DynamicVar> vars;

    void set(string name, DynamicVar v){vars[name] = v;}
    DynamicVar get(string name, DynamicVar defaultValue){if (hasValue(name)) return vars[name]; else return defaultValue;}
    vector<string> getChilds(string parentName)
    {
        if (parentName.size() > 0 && parentName[parentName.size()-1] != '.')
            parentName += ".";
            
        vector<string> ret;
        for (auto &c: vars)
        {
            if (c.first.size() > parentName.size() && c.first.find(parentName) == 0)
            {   
                string tmp = c.first.substr(parentName.size());
                if (tmp.find('.') != string::npos)
                    tmp = tmp.substr(0, tmp.find('.'));

                if (std::count(ret.begin(), ret.end(), tmp) == 0)
                    ret.push_back(tmp);
            }
        }

        return ret;
    }

    bool hasValue(string name){return vars.count(name) > 0;}
    void deleteValue(string name, bool deleteChildsInACascade = false)
    {
        if (hasValue(name)) 
            vars.erase(name);

        vector<string> toDelete;
        if (deleteChildsInACascade)
        {
            for (auto &curr: vars)
            {
                if (curr.first.find(name) == 0)
                {
                    toDelete.push_back(curr.first);
                }
            }
        }

        for (auto &curr: toDelete)
            vars.erase(curr);
    }
    
    void forEachChilds(string parentName, function<void(string, DynamicVar)> f)
    {
        for (auto &c: vars)
        {
            if (c.first != parentName && c.first.find(parentName) == 0)
                f(c.first, c.second);
        }
    }

    future<void> forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel)
    {
        vector <future<void>> pending;

        this->forEachChilds(parentName, [&](string name, DynamicVar value){
            pending.push_back(
                taskerForParallel->enqueue([&](string name, DynamicVar value){
                    f(name, value);
                }, name, value)
            );
        });

        return taskerForParallel->enqueue([&](){
            for (auto &c: pending)
                c.wait();
        });
    }
};

#endif
