#ifndef __HTTPAPI__H__ 
#define __HTTPAPI__H__ 

#include <ApiInterface.h>
#include <messagebus.h>
#include <JSON.h>

namespace API::HTTP{
    class HttpAPI: public ApiInterface{ 
    private:
        string apiId = "HTTPAPI";
        int port = 5023;
        void startListenMessageBus(MessageBus<JsonMaker::JSON> *bus);

    public: 
        HttpAPI(); 
        ~HttpAPI(); 
    public:
        /* ApiInterface implementation */
        string getApiId();
        ClientSendResult notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues);
        ClientSendResult checkAlive(string clientId);
    }; 
}
 
#endif 
