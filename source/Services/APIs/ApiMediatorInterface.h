#ifndef _APIMEDIATORINTERFACE_H_
#define _APIMEDIATORINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include "../../Shared/Misc/DynamicVar.h"
using namespace std;


namespace API
{
    using namespace Shared;
    
    class ApiMediatorInterface
    {
    public:
        virtual future<void> createAlias(string name, string dest) = 0;

        void observeVar(string varName, function<void(string name, string value, void* args)> callback, void* args = NULL, string observerId = "");
        void stopObserve(string observerId);

        //the JsPromise (from ThreadPool) will no be used yet because it must be tested
        /*
        virtual JsPromise* GetVar
         (string name, DynamicVar defaultValue) = 0;
        a = provider.getVar("test");
        a.then((void* r){
            DynamicVar value = (DynamicVar)r;
            
        })
        */


        //return the var name (if a alias is send, returns the correct var name) and the value.
        virtual future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue) = 0;
        virtual future<void> setVar(string name, DynamicVar value) = 0;
        virtual future<void> delVar(string varname) = 0;
        virtual future<vector<string>> getChildsOfVar(string parentName) = 0;

            

    };
};
#endif
