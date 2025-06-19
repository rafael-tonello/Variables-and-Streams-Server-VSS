#include  "httpapi.h" 
 
API::HTTP::HttpAPI::HttpAPI(int httpPort, int httpsPort, DependencyInjectionManager* dim)
{ 
    this->dim = dim;
    this->httpPort = httpPort;
    this->httpsPort = httpsPort;
    this->ctrl = dim->get<ApiMediatorInterface>();
    this->log = dim->get<ILogger>()->getNamedLogger("API::HTTP");
    this->conf = dim->get<Confs>();

    this->conf->listenA("httpApiReturnFullPaths", [&](DynamicVar value){
        this->returnFullPaths = value.getBool();
    });

    initHttpServer();
    //initHttpsServer();
    startListenMessageBus(dim->get<MessageBus<JsonMaker::JSON>>());

    this->ctrl->apiStarted(this);
}
 
API::HTTP::HttpAPI::~HttpAPI() 
{ 
    for (auto &c: servers)
        delete c;

    servers.clear();
}

void API::HTTP::HttpAPI::initHttpServer()
{
    this->initServer(httpPort, false, "", "");
    log.info(string("Http API started a webserver and is listening on port ") + to_string(this->httpPort));
}

void API::HTTP::HttpAPI::initHttpsServer()
{
    //httpApiCertFile httpApiKeyFile
    string certFile = dim->get<Confs>()->getA("httpApiCertFile").getString();
    string keyFile = dim->get<Confs>()->getA("httpApiKeyFile").getString();

    if (certFile == "" || keyFile == "")
    {
        log.error("The HTTPS API is not started because the certFile or keyFile is not setted");
        return;
    }

    this->initServer(httpsPort, true, keyFile, certFile);
    
    log.info(string("Https API started a webserver and is listening on port ") + to_string(this->httpsPort));
}

void API::HTTP::HttpAPI::initServer(int port, bool https, string httpsKey, string httpsPubCert)
{
    auto server = new KWTinyWebServer(port, 
        new WebServerObserverHelper(
            [&](shared_ptr<HttpData> in, shared_ptr<HttpData> out){
                this->onServerRequest(in, out);
            },
            [&](shared_ptr<HttpData>originalRequest, string resource){
                this->onServerWebSocketConnected(originalRequest, resource);
            },
            [](shared_ptr<HttpData>originalRequest, string resource, char* data, unsigned long long dataSize){}
        ),
        {},
        { dim->get<Confs>()->getA("httpDataDir").getString() },
        //dim->get<ThreadPool>(),
        //new ThreadPool(10, 0, "VSSHTTPAPI"),
        NULL,
        https,
        httpsKey,
        httpsPubCert
    );

    //server->__serverName = "Var Streams Server, version ""
    server->setServerInfo(
        "Var Stream Server " 
        + string(dim->get<string>("systemVersion")->c_str()) 
        //+ ", " 
        //+ server->getServerName() 
        //+ " " + server->getServerVersion()
    );

    this->servers.push_back(server);
}

void API::HTTP::HttpAPI::onServerRequest(shared_ptr<HttpData> in, shared_ptr<HttpData> out)
{
    log.debug("request received: "+in->resource);
    
    if (in->method == "GET")
        getVars(in, out);
    else if (in->method == "POST")
        postVar(in, out);
    else if (in->method == "DELETE")
        deleteVar(in, out);
}

void API::HTTP::HttpAPI::getVars(shared_ptr<HttpData> in, shared_ptr<HttpData> out)
{
    string varName = getVarName(in);
    
    auto result = ctrl->getVar(varName, "").get();

    if (result.status == Errors::NoError)
    {

        auto exporter = detectExporter(in);
        
        for (auto &currVar: result.result)
        {
            auto key = std::get<0>(currVar);
            auto value = std::get<1>(currVar);

            if (value.getString() == "")
                continue;
            
            if (!returnFullPaths){
                auto equalPart = getEqualPart(key, varName);
                if (equalPart != "")
                    key = key.substr(equalPart.size());

                if (key.find(".") == 0)
                    key = key.substr(1);
            }


            exporter->add(key, value);
        }
        

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
        out->setContentString(result.status);
    }
}

string API::HTTP::HttpAPI::getEqualPart(string p1, string p2)
{
    string ret = "";
    int i = 0;
    while (i < p1.size() && i < p2.size() && p1[i] == p2[i])
    {
        ret += p1[i];
        i++;
    }
    
    return ret;
}

void API::HTTP::HttpAPI::postVar(shared_ptr<HttpData> in, shared_ptr<HttpData> out)
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
        out->setContentString(result);
    }
    else
    {
        out->httpStatus = 500;
        out->httpMessage = "Internal server error";
        out->setContentString(result);
    }
}

void API::HTTP::HttpAPI::deleteVar(shared_ptr<HttpData> in, shared_ptr<HttpData> out)
{
    string varName = getVarName(in);

    auto result = ctrl->delVar(varName).get();

    if (result == Errors::NoError)
    {
        out->httpStatus = 204;
        out->httpMessage = "No content";
    }
    else if (result == Errors::Error_VariableWithWildCardCantBeSet || result == Errors::Error_VariablesStartedWithUnderscornAreJustForInternal)
    {
        out->httpStatus = 403;
        out->httpMessage = "Forbidden";
        out->setContentString(result);
    }
    else
    {
        out->httpStatus = 500;
        out->httpMessage = "Internal server error";
        out->setContentString(result);
    }
}

string API::HTTP::HttpAPI::getVarName(string resource)
{
    if (resource.size() > 0 && resource[0] == '/')
        resource = resource.substr(1);
    resource = Utils::sr(resource, "/", ".");
    
    return resource;
}

string API::HTTP::HttpAPI::getVarName(shared_ptr<HttpData> in)
{
    auto resource = in->resource;
    if (resource.find("?") != string::npos)
        resource = resource.substr(0, resource.find("?"));

    return getVarName(resource);
}

API::HTTP::IVarsExporter *API::HTTP::HttpAPI::detectExporter(shared_ptr<HttpData>request)
{
    //format via query is mandatory
    if (request->resource.find('?') != string::npos)
    {
        auto query = request->resource.substr(request->resource.find('?')+1);
        if ((query.find("export=json") != string::npos) || (query.find("format=json") != string::npos))
            return new JsonExporter();
        else if ((query.find("export=plain") != string::npos) || (query.find("format=plain") != string::npos))
            return new PlainTextExporter();
    }

    if (PlainTextExporter::checkMimeType(request->accept))
        return new PlainTextExporter();


    return new JsonExporter();
}
 
string API::HTTP::HttpAPI::getApiId()
{
    return this->apiId;
}

void API::HTTP::HttpAPI::onServerWebSocketConnected(shared_ptr<HttpData>originalRequest, string resource)
{
    string addressAsString = to_string((uint64_t)((void*)originalRequest.get()));
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

        for (auto &server: servers)
            server->sendWebSocketData(wsConnections[clientId], (char*)(data.c_str()), data.size(), true);
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
        if (this->httpPort > -1 || this->httpPort > -1)
        {
            result.setString("name", "HTTP - Http API");
            result.setString("access", getListeningInfo());
        }
    });
}

string API::HTTP::HttpAPI::getListeningInfo()
{   
    string ret = "";
    if (httpPort > -1)
        ret += "TCP/"+to_string(httpPort) + "(http)";
    
    if (httpsPort > -1)
    {
        if (ret != "") ret += ", ";
        ret += "TCP/"+to_string(httpsPort) + "(https)";
    }

    if (ret == "")
        ret = "Error - No port opened";

    return ret;
}