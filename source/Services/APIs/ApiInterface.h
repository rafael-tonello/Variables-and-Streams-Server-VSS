#ifndef _APIINTERFACE_H_
#define _APIINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include "../../Shared/Misc/DynamicVar.h"

using namespace Shared;
using namespace std;
namespace API
{

    using namespace Shared;
    
    enum ClientSendResult {LIVE, DISCONNECTED};

    class ApiInterface
    {
    public:
        virtual string getApiId() = 0;
        virtual ClientSendResult notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues) = 0;
        virtual ClientSendResult checkAlive(string clientId) = 0;

    };
};
#endif
