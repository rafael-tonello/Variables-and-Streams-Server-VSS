#include  "httpapi.h" 
 
API::HTTP::HttpAPI::HttpAPI(int port, DependencyInjectionManager* dim)
{ 
    this->port = port;
    this->ctrl = dim->get<ApiMediatorInterface>();
    server = new KWTinyWebServer(this->port, new WebServerObserverHelper(
        [&](HttpData* in, HttpData* out){
            this->onServerRequest(in, out);
        }),
        {}
    );

    startListenMessageBus(dim->get<MessageBus<JsonMaker::JSON>>());
}


 
API::HTTP::HttpAPI::~HttpAPI() 
{ 
    delete server;
} 

void API::HTTP::HttpAPI::onServerRequest(HttpData* in, HttpData* out)
{
    string varName = in->resource;
    if (varName.size() > 0 && varName[0] == '/')
        varName = varName.substr(1);
    varName = Utils::sr(varName, "/", ".");
    
    auto varsResult = ctrl->getVar(varName, "").get();

    auto exporter = detectExporter(in);
    
    for (auto &currVar: varsResult)
        exporter->add(std::get<0>(currVar) + "._value", std::get<1>(currVar));
    

    out->contentType = exporter->getMimeType();
    out->setContentString(exporter->toString());

    delete exporter;

}

API::HTTP::IVarsExporter *API::HTTP::HttpAPI::detectExporter(HttpData *request)
{
    if (PlainTextExporter::checkMimeType(request->contentType))
        return new PlainTextExporter();

    return new JsonExporter();
}
 
string API::HTTP::HttpAPI::getApiId()
{
    return this->apiId;
}

API::ClientSendResult API::HTTP::HttpAPI::notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
{
    return API::ClientSendResult::DISCONNECTED;
}

API::ClientSendResult API::HTTP::HttpAPI::checkAlive(string clientId)
{
    return API::ClientSendResult::DISCONNECTED;
}

void API::HTTP::HttpAPI::startListenMessageBus(MessageBus<JsonMaker::JSON> *bus)
{
    bus->listen("discover.startedApis", [&](string message, JsonMaker::JSON args, JsonMaker::JSON &result){
        if (this->port > -1)
        {
            result.setString("name", "HTTP - Http API");
            result.setString("access", getListeningInfo());
        }
    });
}

string API::HTTP::HttpAPI::getListeningInfo()
{
    if (this->port > -1)
        return "TCP/"+to_string(port);
    else
        return "Error - No opened port";
}