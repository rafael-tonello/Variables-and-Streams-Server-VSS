#ifndef __APIMEDIATORINTERFACEMOCK__H__
#define __APIMEDIATORINTERFACEMOCK__H__ 

#include <ApiMediatorInterface.h>
#include <future>

using namespace std;
using namespace API;

class ApiMediatorInterfaceMock: public ApiMediatorInterface{
public: /** ApiMediatorInterface */

    future<GetVarResult> getVar(string name, DynamicVar defaultValue)
    {
        return getVarF(name, defaultValue);
    }

    future<Errors::Error> setVar(string name, DynamicVar value)
    {
        return setVarF(name, value);
    }

    future<Errors::Error> delVar(string varname)
    {
        return delVarF(varname);
    }

    future<Errors::ResultWithStatus<vector<string>>> getChildsOfVar(string parentName)
    {
        return getChildsOfVarF(parentName);
    }


    
    virtual future<Errors::Error> lockVar(string varName, uint maxTimeOut_ms = UINT_MAX)
    {
        return lockVarF(varName, maxTimeOut_ms);
    }

    virtual future<Errors::Error> unlockVar(string varName)
    {
        return unlockVarF(varName);
    }

    virtual bool isVarLocked(string varName)
    {
        return isVarLockedF(varName);
    }

    
    virtual void apiStarted(ApiInterface *api)
    {
        apiStartedF(api);
    }
    virtual string clientConnected(string clientId, ApiInterface* api, int &observingVarsCount)
    {
        return clientConnectedF(clientId, api, observingVarsCount);
    }
    virtual void observeVar(string varName, string clientId, string customIdsAndMetainfo, ApiInterface* api)
    {
        observeVarF(varName, clientId, customIdsAndMetainfo, api);
    }
    virtual void stopObservingVar(string varName, string clientId, string customIdsAndMetainfo, ApiInterface* api)
    {
        stopObservingVarF(varName, clientId, customIdsAndMetainfo, api);
    }
    virtual string getSystemVersion()
    {
        return getSystemVersionF();
    }
    //virtual void stopObservingVar(string varName, string clientId, ApiInterface* api);

    map<string, DynamicVar> vars;

public: /** method mocks */
    typedef function<future<GetVarResult>(string, DynamicVar)> getVarFunc;
    getVarFunc getVarF = [=, this](string name, DynamicVar defaultValue) -> future<GetVarResult>{
        return std::async(std::launch::async, [=, this](){
            //return a default result
            GetVarResult result;
            result.status = Errors::Error(Errors::NoError);
            result.result = VarList();
            if (name.find("*") != string::npos)
            {
                string nameWithNotWildcard = name.substr(0, name.find("*"));
                //return all vars
                for (auto &c: vars){
                    if (c.first.find(nameWithNotWildcard) == 0)
                        result.result.push_back(make_tuple(c.first, c.second));
                }
            }
            else if (vars.count(name) > 0)
            {
                result.result.push_back(make_tuple(name, vars[name]));
            }
            

            if (result.result.size() == 0)
                result.result.push_back(make_tuple(name, defaultValue));
                

            return result;
        });
    };

    typedef function<future<Errors::Error>(string, DynamicVar)> setVarFunc;
    setVarFunc setVarF = [=, this](string name, DynamicVar value) -> future<Errors::Error>{
        return std::async(std::launch::async, [=, this](){
            //return a default result
            vars[name] = value;
            return Errors::Error(Errors::NoError);
        });
    };

    typedef function<future<Errors::Error>(string)> delVarFunc;
    delVarFunc delVarF = [](string) -> future<Errors::Error>{
        return std::async(std::launch::async, [](){
            return Errors::Error(Errors::NoError);
        });
    };

    typedef function<future<Errors::ResultWithStatus<vector<string>>>(string)> getChildsOfVarFunc;
    getChildsOfVarFunc getChildsOfVarF = [](string) -> future<Errors::ResultWithStatus<vector<string>>>{
        return std::async(std::launch::async, [](){
            //return a default result
            Errors::ResultWithStatus<vector<string>> result;
            result.status = Errors::Error(Errors::NoError);
            result.result = vector<string>();
            return result;
        });
    };

    typedef function<future<Errors::Error>(string, uint)> lockVarFunc;
    lockVarFunc lockVarF = [](string, uint) -> future<Errors::Error>{
        return std::async(std::launch::async, [](){
            //return a default result
            return Errors::Error(Errors::NoError);
        });
    };

    typedef function<future<Errors::Error>(string)> unlockVarFunc;
    unlockVarFunc unlockVarF = [](string) -> future<Errors::Error>{
        return std::async(std::launch::async, [](){
            //return a default result
            return Errors::Error(Errors::NoError);
        });
    };

    typedef function<bool(string)> isVarLockedFunc;
    isVarLockedFunc isVarLockedF = [](string) -> bool {
        return false; // return a default value
    };

    typedef function<void(ApiInterface*)> apiStartedFunc;
    apiStartedFunc apiStartedF = [](ApiInterface*) {
        // do nothing by default
    };

    typedef function<string(string, ApiInterface*, int&)> clientConnectedFunc;
    clientConnectedFunc clientConnectedF = [](string, ApiInterface*, int&) -> string {
        return ""; // return a default value
    };

    typedef function<void(string, string, string, ApiInterface*)> observeVarFunc;
    observeVarFunc observeVarF = [](string, string, string, ApiInterface*) {
        // do nothing by default
    };

    typedef function<void(string, string, string, ApiInterface*)> stopObservingVarFunc;
    stopObservingVarFunc stopObservingVarF = [](string, string, string, ApiInterface*) {
        // do nothing by default
    };

    typedef function<string()> getSystemVersionFunc;
    getSystemVersionFunc getSystemVersionF = []() -> string {
        return "1.0.0"; // return a default version
    };
public:
    void setOnGetVar(getVarFunc func){ this->getVarF = func; }
    void setOnSetVar(setVarFunc func){ this->setVarF = func; }
    void setOnDelVar(delVarFunc func){ this->delVarF = func; }
    void setOnGetChildsOfVar(getChildsOfVarFunc func){ this->getChildsOfVarF = func; }
    void setOnLockVar(lockVarFunc func){ this->lockVarF = func; }
    void setOnUnlockVar(unlockVarFunc func){ this->unlockVarF = func; }
    void setOnIsVarLocked(isVarLockedFunc func){ this->isVarLockedF = func; }
    void setOnApiStarted(apiStartedFunc func){ this->apiStartedF = func; }
    void setOnClientConnected(clientConnectedFunc func){ this->clientConnectedF = func; }
    void setOnObserveVar(observeVarFunc func){ this->observeVarF = func; }
    void setOnStopObservingVar(stopObservingVarFunc func){ this->stopObservingVarF = func; }
    void setOnGetSystemVersion(getSystemVersionFunc func){ this->getSystemVersionF = func; }
};

#endif
