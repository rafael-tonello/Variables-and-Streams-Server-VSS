#ifndef __HTTPAPI__H__ 
#define __HTTPAPI__H__ 

#include <ApiInterface.h>
#include <messagebus.h>
#include <JSON.h>
#include <ApiMediatorInterface.h>
#include <KWTinyWebServer.h>
#include <WebServerObserverHelper.h>
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
        bool returnFullPaths = false;
        void startListenMessageBus(MessageBus<JsonMaker::JSON> *bus);
        ApiMediatorInterface* ctrl;
        vector<KWShared::KWTinyWebServer*> servers;
        NLogger log;
        DependencyInjectionManager* dim;
        Confs *conf;

        void onServerRequest(shared_ptr<HttpData> in, shared_ptr<HttpData> out);
        IVarsExporter *detectExporter(shared_ptr<HttpData>request);

        map<string, shared_ptr<HttpData>> wsConnections;

        string getVarName(shared_ptr<HttpData> in);
        string getVarName(string resource);
        void getVars(shared_ptr<HttpData> in, shared_ptr<HttpData> out);
        void postVar(shared_ptr<HttpData> in, shared_ptr<HttpData> out);
        void deleteVar(shared_ptr<HttpData> in, shared_ptr<HttpData> out);

        void onServerWebSocketConnected(shared_ptr<HttpData>originalRequest, string resource);

        void initHttpServer();
        void initHttpsServer();
        void initServer(int port, bool https, string httpsKey, string httpsPubCert);

        static string getEqualPart(string p1, string p2);

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
