#ifndef _APIMEDIATORINTERFACE_H_
#define _APIMEDIATORINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include "../../Shared/Misc/DynamicVar.h"

using namespace Shared;
using namespace std;
namespace API
{
    typedef function<void(string name, DynamicVar value, void* args, string id)> observerCallback;

    using namespace Shared;
    
    class ApiMediatorInterface
    {
    public:
        virtual future<void> createAlias(string name, string dest) = 0;
        virtual future<string> getAliasValue(string aliasName) = 0;
        virtual future<void> deleteAlias(string aliasName) = 0;

        virtual string observeVar(string varName, observerCallback callback, void* args = NULL, string observerId = "") = 0;
        virtual void stopObservingVar(string observerId) = 0;

        //return the var name (if a alias is send, returns the correct var name) and the value (returna vector because you can request a var like "a.b.c.*").
        virtual future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue) = 0;
        virtual future<void> setVar(string name, DynamicVar value) = 0;
        virtual future<void> delVar(string varname) = 0;
        virtual future<vector<string>> getChildsOfVar(string parentName) = 0;

        
        virtual future<void> lockVar(string varName) = 0;
        virtual future<void> unlockVar(string varName) = 0;

    };
};
#endif
