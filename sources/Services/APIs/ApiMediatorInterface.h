#ifndef _APIMEDIATORINTERFACE_H_
#define _APIMEDIATORINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <DynamicVar.h>
#include "ApiInterface.h"

using namespace std;
namespace API
{
    typedef function<void(string name, DynamicVar value, void* args, string id)> observerCallback;
    
    class ApiMediatorInterface
    {
    public:
        //return the var name (if a alias is send, returns the correct var name) and the value (returna vector because you can request a var like "a.b.c.*").
        virtual future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue) = 0;
        virtual future<void> setVar(string name, DynamicVar value) = 0;
        virtual future<void> delVar(string varname) = 0;
        virtual future<vector<string>> getChildsOfVar(string parentName) = 0;

        
        virtual future<void> lockVar(string varName) = 0;
        virtual future<void> unlockVar(string varName) = 0;
        
        virtual void apiStarted(ApiInterface *api) = 0;
        virtual string clientConnected(string clientId, ApiInterface* api, int &observingVarsCount) = 0;
        virtual void observeVar(string varName, string clientId, ApiInterface* api) = 0;
        virtual void stopObservingVar(string varName, string clientId, ApiInterface* api) = 0;

    };
};
#endif
