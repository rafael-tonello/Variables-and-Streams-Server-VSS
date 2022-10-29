#include  "httpapi.h" 
 
API::HTTP::HttpAPI::HttpAPI() 
{ 
     
} 
 
API::HTTP::HttpAPI::~HttpAPI() 
{ 
     
} 
 
string API::HTTP::HttpAPI::getApiId()
{
    return this->apiId;
}

API::ClientSendResult API::HTTP::HttpAPI::notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
{

}

API::ClientSendResult API::HTTP::HttpAPI::checkAlive(string clientId)
{

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
