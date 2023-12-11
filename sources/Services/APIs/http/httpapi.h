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
#include <Confs.h>

namespace API::HTTP{
    using namespace KWShared;
    class HttpAPI: public ApiInterface{ 
    private:
        string apiId = "HTTPAPI";
        int httpPort; // = 5023;
        int httpsPort; // = 5023;
        void startListenMessageBus(MessageBus<JsonMaker::JSON> *bus);
        ApiMediatorInterface* ctrl;
        vector<KWShared::KWTinyWebServer *> servers;
        NLogger log;
        DependencyInjectionManager* dim;

        void onServerRequest(HttpData* in, HttpData* out);
        IVarsExporter *detectExporter(HttpData *request);

        map<string, HttpData*> wsConnections;

        string getVarName(HttpData* in);
        string getVarName(string resource);
        void getVars(HttpData* in, HttpData* out);
        void postVar(HttpData* in, HttpData* out);

        void onServerWebSocketConnected(HttpData *originalRequest, string resource);

        void initHttpServer();
        void initHttpsServer();
        void initServer(int port, bool https, string httpsKey, string httpsPubCert);

    public: 
        HttpAPI(int httpPort, int httpsPort, DependencyInjectionManager *dim); 
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
