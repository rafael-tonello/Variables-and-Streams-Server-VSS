#ifndef _APIINTERFACE_H_
#define _APIINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <DynamicVar.h>

using namespace std;
namespace API
{
    
    enum ClientSendResult {LIVE, DISCONNECTED};

    class ApiInterface
    {
    public:
        virtual string getApiId() = 0;
        virtual ClientSendResult notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues) = 0;
        virtual ClientSendResult checkAlive(string clientId) = 0;
        virtual string getListeningInfo() = 0;

        /*
            SOme rules (change to implement via interfaces)
            1) notify controller by use of 'apiStarted' method
            2) reply the message 'discover.startedApis'. This message will come via MessageBus.
                {
                    "name":"api name",
                    "access": "TCP/port"
                }
        */

    };
};
#endif
