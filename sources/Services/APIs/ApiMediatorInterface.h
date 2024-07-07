#ifndef _APIMEDIATORINTERFACE_H_
#define _APIMEDIATORINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <DynamicVar.h>
#include "ApiInterface.h"
#include <errors.h>
#include <tuple>
#include <limits.h>

using namespace std;
namespace API
{
    typedef function<void(string name, DynamicVar value, void* args, string id)> observerCallback;
    using VarList = vector<tuple<std::string, DynamicVar>>;
    using GetVarResult = Errors::ResultWithStatus<VarList>;

    class ApiMediatorInterface
    {
    public:
        //return the var name (if a alias is send, returns the correct var name) and the value (returna vector because you can request a var like "a.b.c.*").
        virtual future<GetVarResult> getVar(string name, DynamicVar defaultValue) = 0;
        virtual future<Errors::Error> setVar(string name, DynamicVar value) = 0;
        virtual future<Errors::Error> delVar(string varname) = 0;
        virtual future<Errors::ResultWithStatus<vector<string>>> getChildsOfVar(string parentName) = 0;

        
        virtual future<Errors::Error> lockVar(string varName, uint maxTimeOut_ms = UINT_MAX) = 0;
        virtual future<Errors::Error> unlockVar(string varName) = 0;
        virtual bool isVarLocked(string varName) = 0;
        
        virtual void apiStarted(ApiInterface *api) = 0;
        virtual string clientConnected(string clientId, ApiInterface* api, int &observingVarsCount) = 0;
        virtual void observeVar(string varName, string clientId, string customIdsAndMetainfo, ApiInterface* api) = 0;
        //virtual void stopObservingVar(string varName, string clientId, ApiInterface* api) = 0;
        virtual void stopObservingVar(string varName, string clientId, string customIdsAndMetainfo, ApiInterface* api) = 0;
        virtual string getSystemVersion() = 0;

    };
}
#endif
