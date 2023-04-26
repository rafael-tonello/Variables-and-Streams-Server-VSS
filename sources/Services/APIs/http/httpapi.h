#ifndef __HTTPAPI__H__ 
#define __HTTPAPI__H__ 

#include <ApiInterface.h>
#include <messagebus.h>
#include <JSON.h>
#include <ApiMediatorInterface.h>
#include <KWTinyWebServer.h>
#include <KwWebServer/sources/helpers/WebServerObserverHelper.h>
#include <utils.h>
#include <ivarsexporter.h>
#include <jsonexporter.h>
#include <plaintextexporter.h>
#include <dependencyInjectionManager.h>
#include <logger.h>

namespace API::HTTP{
    using namespace KWShared;
    class HttpAPI: public ApiInterface{ 
    private:
        string apiId = "HTTPAPI";
        int port; // = 5023;
        void startListenMessageBus(MessageBus<JsonMaker::JSON> *bus);
        ApiMediatorInterface* ctrl;
        KWShared::KWTinyWebServer *server;
        NLogger log;

        void onServerRequest(HttpData* in, HttpData* out);
        IVarsExporter *detectExporter(HttpData *request);

        string getVarName(HttpData* in);
        void getVars(HttpData* in, HttpData* out);
        void postVar(HttpData* in, HttpData* out);

    public: 
        HttpAPI(int port, DependencyInjectionManager *dim); 
        ~HttpAPI(); 
    public:
        /* ApiInterface implementation */
        string getApiId();
        ClientSendResult notifyClient(string clientId, vector<tuple<string, string, DynamicVar>> varsnamesMetadataAndValues);
        ClientSendResult checkAlive(string clientId);
        string getListeningInfo();
    }; 
}
 
#endif 
