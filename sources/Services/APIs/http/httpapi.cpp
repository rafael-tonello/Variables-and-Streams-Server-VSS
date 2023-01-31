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

string API::HTTP::HttpAPI::getVarName(HttpData* in)
{
    string varName = in->resource;
    if (varName.size() > 0 && varName[0] == '/')
        varName = varName.substr(1);
    varName = Utils::sr(varName, "/", ".");
    
    return varName;
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