#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include "../Shared/Misc/Observable.h";
#include "../Shared/Misc/DynamicVar.h";
#include "../Services/VarSystem/FileVars.h"
#include "../Shared/ThreadPool/ThreadPool.h"
#include "../Services/APIs/ApiMediatorInterface.h"
#include "../Services/VarSystem/FileVars.h"
#include "../Shared/Confs/Confs.h"
#include "../Shared/DependencyInjectionManager/dependencyInjectionManager.h"

#ifdef __TESTING__
    #include <tester.h>
#endif

using namespace Shared;
using namespace std;    

namespace Controller{
    typedef function<void(string name, DynamicVar value, void* args, string id)> observerCallback;

    struct ResolveVarNameResult{
        string search;
        string destination;
        bool searchIsAnAlias;
    };
    
    struct VarObserverInfo{
        string id = "";
        void* args;
        observerCallback fun;

        //this variable is used when an observer is added to a variabel using an alias name. It is used to notify clients by use of the alias name
        string aliasName;
    };

    struct VarNode{
        VarNode* parent = NULL;
        string name = "";
        string fullName = "";
        map<string, VarNode> childs;
        vector<VarObserverInfo> observers;
    };

    class TheController: public Observable, public API::ApiMediatorInterface
    {
    private:
        #ifdef __TESTING__
            public:
        #endif

        ThreadPool tasker;

        VarNode rootNode;

        FileVars *db = NULl;

        Config *confs;

        //FileVars sVarSystem("vars", true);
        
        mutex varsObserversMutex;
        map<string, vector<VarObserverInfo>> varsObservers;

        VarNode* _findNode(string name, VarNode* curr, bool createNewNodes = true);

        ResolveVarNameResult _resolveVarName(string aliasOrVarName);

        string _createId();

        //a list of observers and their respective VarNode. This map is used to facilidate the work of function 'stopObservingVar'
        map<string, VarNode*> observersShorcut;



        future<void> internalSetVar(string name, DynamicVar value);

        DynamicVar getVarInternalFlag(string vName, string flagName, DynamicVar defaultValue);
        void setVarInternalFlag(string vName, string flagName, DynamicVar value);

    public:
        TheController(DependencyInjectionManager* dim);
        ~TheController();

        //returned the literal value (a variable name) of an alias

        future<void> createAlias(string name, string dest);
        future<void> deleteAlias(string aliasName);
        future<string> getAliasValue(string aliasName);

        string observeVar(string varName, observerCallback callback, void* args = NULL, string observerId = "");
        void stopObservingVar(string observerId);

        //return the var name (if a alias is send, returns the correct var name) and the value (returna vector because you can request a var like "a.b.c.*").
        future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue);
        future<void> setVar(string name, DynamicVar value);
        future<void> delVar(string varname);
        future<vector<string>> getChildsOfVar(string parentName);
        future<void> lockVar(string varName);
        future<void> unlockVar(string varName);

    };
}
#endif;