#ifndef __CONTROLLER_CLIENTHELPERTEST__H__ 
#define __CONTROLLER_CLIENTHELPERTEST__H__ 

#include <tester.h>
#include <StorageInterface.h>
#include <Controller_ClientHelper.h>

class Controller_ClientHelperTester: public Tester { 
public: 
    Controller_ClientHelperTester(); 
public:
    /* Tester class */
    vector<string> getContexts();
    void run(string context);
};

class TmpDB: public StorageInterface
{
public:
    map<string, DynamicVar> vars;

    bool set(string name, DynamicVar v){vars[name] = v;}
    DynamicVar get(string name, DynamicVar defaultValue){if (exists(name)) return vars[name]; else return defaultValue;}
    vector<string> getChilds(string parentName)
    {
        vector<string> ret;
        for (auto &c: vars)
        {
            if (c.first.find (parentName) == 0)
                ret.push_back(c.first);
        }

        return ret;
    }

    bool exists(string name){return vars.count(name) > 0;}
    bool del(string name){if (exists(name)) vars.erase(name);}
    void forEach(string parentName, function<void(DynamicVar)> f){for (auto &c: vars) f(c.second);}
    future<void> forEach_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel)
    {
        vector <future<void>> pending;

        for (auto &c: vars) 
        {
            pending.push_back(
                taskerForParallel->enqueue([&](string name, DynamicVar value){
                    f(name, value);
                }, c.first, c.second)
            );
        }

        return taskerForParallel->enqueue([](){
            for (auto &c: pending)
                c.wait();
        });
    }
};

class ApiInterfaceTmp: public ApiInterface
{
public:
    string getApiId()
    {
        return "tmpApiId";
    }

    ClientSendResult notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
    {
        return ClientSendResult::LIVE;
    }

    ClientSendResult checkAlive(string clientId)
    {
        return ClientSendResult::LIVE;
    }
};
 
#endif 
