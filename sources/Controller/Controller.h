#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <DynamicVar.h>
#include <ThreadPool.h>
#include <ApiMediatorInterface.h>
#include <Confs.h>
#include <dependencyInjectionManager.h>
#include <StorageInterface.h>
#include <ApiInterface.h>
#include <utils.h>
#include <logger.h>
#include "Internal/Controller_ClientHelper.h"
#include "Internal/Controller_VarHelper.h"
#include <errors.h>

#ifdef __TESTING__
    #include <tester.h>
#endif

using namespace Shared;
using namespace std;
using namespace API;

namespace Controller{
    


    typedef function<void(string name, DynamicVar value, void* args, string id)> observerCallback;
    
    struct VarObserverInfo{
        string id = "";
        void* args;
        observerCallback fun;

        //this variable is used when an observer is added to a variabel using an alias name. It is used to notify clients by use of the alias name
        string aliasName;
    };

    class TheController: public API::ApiMediatorInterface
    {
    private:
        #ifdef __TESTING__
            public:
        #endif

        mutex observerMutex;
        ThreadPool *tasker;
        Shared::Config *confs;
        StorageInterface* db;
        ILogger *log;
        int64_t maxTimeWaitingClient_seconds = 12*60*60;

        map<string, ApiInterface*> apis;

        string _createUniqueId();

        void updateClientAboutObservatingVars(Controller_ClientHelper controller_ClientHelper);
        void notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues);
        void deleteClient(Controller_ClientHelper client);

        //checks the amount of time that client is disconnected and remove them from the system
        void checkClientLiveTime(Controller_ClientHelper client);

        future<void> internalSetVar(string name, DynamicVar value);

        DynamicVar getVarInternalFlag(string vName, string flagName, DynamicVar defaultValue);
        void setVarInternalFlag(string vName, string flagName, DynamicVar value);
        void notifyVarModification(string varName, DynamicVar value);
        void notifyParentGenericObservers(string varName, string changedVarName, DynamicVar value);
        void notifyClientsAboutVarChange(vector<string> clients, string changedVarName, DynamicVar value);
    public:
        TheController(DependencyInjectionManager* dim);
        ~TheController();

    /* ApiMediatorIntgerface interface*/
    public: 

        void apiStarted(ApiInterface *api);
        string clientConnected(string clientId, ApiInterface* api, int &observingVarsCount);
        void observeVar(string varName, string clientId, ApiInterface* api);
        void stopObservingVar(string varName, string clientId, ApiInterface* api);

        //return the var name (if a alias is send, returns the correct var name) and the value (returna vector because you can request a var like "a.b.c.*").
        future<GetVarResult> getVar(string name, DynamicVar defaultValue);
        future<Errors::Error> setVar(string name, DynamicVar value);
        future<Errors::Error> delVar(string varname);

        
        future<Errors::Error> lockVar(string varName);
        future<Errors::Error> unlockVar(string varName);
        future<vector<string>> getChildsOfVar(string parentName);
        
    };
}
#endif
