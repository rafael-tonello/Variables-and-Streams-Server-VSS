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
#include "../Shared/ThreadPool/JsPromise.h"
#include "../Services/APIs/ApiMediatorInterface.h"

using namespace Shared;
using namespace std;    

typedef function<void(string name, DynamicVar value, void* args, string id)> observerCallback;


namespace HomeAut { namespace Controller {
    
    string MESSAGE_VAR_SET = "setVar";
    string MESSAGE_ALIAS_SET = "setAlias";


    struct VarObserverInfo{
        string id = "";
        void* args;
        observerCallback fun;
    };

    enum VarType {NormalVar, Alias};
    struct VarNode{
        VarNode* parent = NULL;
        string name = "";
        string fullName = "";
        DynamicVar value;
        VarType type = VarType::NormalVar;
        map<string, VarNode> childs;
        vector<VarObserverInfo> observers;
        bool valueSetted = false;
    };

    class Controller: public Observable, public API::ApiMediatorInterface
    {
    private:
        ThreadPool tasker;

        VarNode rootNode;

        //FileVars sVarSystem("vars", true);
        
        mutex varsObserversMutex;
        map<string, vector<VarObserverInfo>> varsObservers;

        VarNode* _findNode(string name, VarNode* curr, bool createNewNodes = true);

        string _resolveVarName(string aliasOrVarName);

        string _createId();

        //a list of observers and their respective VarNode. This map is used to facilidate the work of function 'stopObservingVar'
        map<string, VarNode*> observersShorcut;

    public:
        Controller();

        //returned the literal value (a variable name) of an alias
        string getAliasValue(string aliasName);

        future<void> createAlias(string name, string dest);
        string observeVar(string varName, observerCallback callback, void* args = NULL, string observerId = "");
        void stopObservingVar(string observerId);

        //the JsPromise (from ThreadPool) will no be used yet because it must be tested
        /*
        virtual JsPromise* GetVar
         (string name, DynamicVar defaultValue) = 0;
        a = provider.getVar("test");
        a.then((void* r){
            DynamicVar value = (DynamicVar)r;
            
        })
        */


        //return the var name (if a alias is send, returns the correct var name) and the value (returna vector because you can request a var like "a.b.c.*").
        future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue);
        future<void> setVar(string name, DynamicVar value);
        future<void> delVar(string varname);
        future<vector<string>> getChildsOfVar(string parentName);

    };
}}
#endif;