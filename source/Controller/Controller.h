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

using namespace Shared;
using namespace std;
namespace HomeAut { namespace Controller {
    string MESSAGE_VAR_SET = "setVar";
    string MESSAGE_VAR_SET = "setAlias";

    struct VarObserverInfo{
        string id = "";
        void* args;
        function<void(string name, DynamicVar value, void* args)> fun;
    };

    class Controller: public Observable
    {
    private:
        ThreadPool tasker;

        FileVars sVarSystem("vars", true);
        
        mutex varsObserversMutex;
        map<string, vector<VarObserverInfo>> varsObservers;

        string resolveVarName(string aliasOrVarName);
    public:
        Controller();
        //creates or change a variable
        void setVar(string name, DynamicVar variable);
        
        //create an alias (a shortcut) to a variable (or to another alias)
        void setAlias(string aliasName, string varName);

        //returned the literal value (a variable name) of an alias
        string getAliasValue(string aliasName);

        //start to observate a variable
        void observeVar(string varName, function<void(string name, string value, void* args)> callback, void* args = NULL, string id = "");
        //stop observate variable
        void stopObserve(string id);

    };
}}
#endif;