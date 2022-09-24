#ifndef __API_INTERFACE_TMP__
#define __API_INTERFACE_TMP__

#include<ApiInterface.h>
#include <tuple>
#include <functional>

using namespace std;
using namespace API;

class ApiInterfaceTmp: public ApiInterface
{
private:
    function<string()> getApiIdFunction = [](){return "tempApiId";};
    function<ClientSendResult(string clientId, vector<tuple<string, DynamicVar>> varsAndValues)> notifyClientFunction = [](string clientId, vector<tuple<string, DynamicVar>> varsAndValues){ return ClientSendResult::LIVE;};
    function<ClientSendResult(string clientId)> checkAliveFunction = [](string clientId){return ClientSendResult::LIVE;};
public:

    void setFunction_getApiId (function<string()> f)
    {
        getApiIdFunction = f;
    }
    
    void setFunction_notifyClient(function<ClientSendResult(string clientId, vector<tuple<string, DynamicVar>> varsAndValues)> f)
    {
        notifyClientFunction = f;
    }

    void setFunction_checkAlive(function<ClientSendResult(string clientId)> f)
    {
        checkAliveFunction = f;
    }

    string getApiId()
    {
        return getApiIdFunction();
    }

    ClientSendResult notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
    {
        return notifyClientFunction(clientId, varsAndValues);
    }

    ClientSendResult checkAlive(string clientId)
    {
        return checkAliveFunction(clientId);
    }
};

#endif