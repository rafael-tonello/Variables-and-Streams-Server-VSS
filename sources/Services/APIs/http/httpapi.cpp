#include  "httpapi.h" 
 
API::HTTP::HttpAPI::HttpAPI(int port, DependencyInjectionManager* dim)
{ 
    this->port = port;
    this->ctrl = dim->get<ApiMediatorInterface>();
    this->log = dim->get<ILogger>()->getNamedLogger("API::HTTP");


    server = new KWTinyWebServer(this->port, 
        new WebServerObserverHelper(
            [&](HttpData* in, HttpData* out){
                this->onServerRequest(in, out);
            },
            [&](HttpData *originalRequest, string resource){
                this->onServerWebSocketConnected(originalRequest, resource);
            },
            [](HttpData *originalRequest, string resource, char* data, unsigned long long dataSize){}
        ),
        {},
        { dim->get<Confs>()->getA("httpDataDir").getString() }
    );
    log.info(string("Http API started a webserver and is listening on port ") + to_string(this->port));

    startListenMessageBus(dim->get<MessageBus<JsonMaker::JSON>>());

    this->ctrl->apiStarted(this);
}


 
API::HTTP::HttpAPI::~HttpAPI() 
{ 
    delete server;
} 

void API::HTTP::HttpAPI::onServerRequest(HttpData* in, HttpData* out)
{
    log.debug("request received: "+in->resource);
    
    if (in->method == "GET")
        getVars(in, out);
    else if (in->method == "POST")
        postVar(in, out);
}

void API::HTTP::HttpAPI::getVars(HttpData* in, HttpData* out)
{
    string varName = getVarName(in);
    
    auto result = ctrl->getVar(varName, "").get();

    if (result.errorStatus == Errors::NoError)
    {

        auto exporter = detectExporter(in);
        
        for (auto &currVar: result.result)
            exporter->add(std::get<0>(currVar), std::get<1>(currVar));
        

        out->contentType = exporter->getMimeType();
        out->setContentString(exporter->toString());
        out->httpStatus = 200;
        out->httpMessage = "OK";

        delete exporter;
    }
    else
    {
        out->httpStatus = 400;
        out->httpMessage = "Bad request";
        out->setContentString(result.errorStatus.message);
    }
}

void API::HTTP::HttpAPI::postVar(HttpData* in, HttpData* out)
{
    string varName = getVarName(in);

    auto result = ctrl->setVar(varName, in->getContentString()).get();

    if (result == Errors::NoError)
    {
        out->httpStatus = 204;
        out->httpMessage = "No content";
    }
    else if (result == Errors::Error_VariableWithWildCardCantBeSet || result == Errors::Error_VariablesStartedWithUnderscornAreJustForInternal)
    {
        out->httpStatus = 403;
        out->httpMessage = "Forbidden";
        out->setContentString(result.message);
    }
    else
    {
        out->httpStatus = 500;
        out->httpMessage = "Internal server error";
        out->setContentString(result.message);
    }
}

string API::HTTP::HttpAPI::getVarName(string resource)
{
    if (resource.size() > 0 && resource[0] == '/')
        resource = resource.substr(1);
    resource = Utils::sr(resource, "/", ".");
    
    return resource;
}

string API::HTTP::HttpAPI::getVarName(HttpData* in)
{
    return getVarName(in->resource);
}

API::HTTP::IVarsExporter *API::HTTP::HttpAPI::detectExporter(HttpData *request)
{
    if (PlainTextExporter::checkMimeType(request->accept))
        return new PlainTextExporter();

    return new JsonExporter();
}
 
string API::HTTP::HttpAPI::getApiId()
{
    return this->apiId;
}

void API::HTTP::HttpAPI::onServerWebSocketConnected(HttpData *originalRequest, string resource)
{
    string addressAsString = to_string((uint64_t)((void*)originalRequest));
    wsConnections[addressAsString] = originalRequest;

    auto varName = getVarName(resource);
    
    this->ctrl->observeVar(varName, addressAsString, "", this);
}

API::ClientSendResult API::HTTP::HttpAPI::notifyClient(string clientId, vector<tuple<string, string, DynamicVar>> varsnamesMetadataAndValues)
{
    if (!wsConnections.count(clientId))
        return API::ClientSendResult::DISCONNECTED;

    for (auto &c: varsnamesMetadataAndValues)
    {
        auto varName = std::get<0>(c);
        auto varValue = std::get<2>(c).getString();

        string data = varName + "=" + varValue;

        this->server->sendWebSocketData(wsConnections[clientId], (char*)(data.c_str()), data.size(), true);
    }

    return API::ClientSendResult::LIVE;
}

API::ClientSendResult API::HTTP::HttpAPI::checkAlive(string clientId)
{
    return wsConnections.count(clientId) > 0 ? API::ClientSendResult::DISCONNECTED : API::ClientSendResult::LIVE;
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
        return "Error - No port opened";
}