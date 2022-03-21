#include  "Controller_ClientHelper.h" 
 
void Controller_ClientHelper::initialize()
{
    //try find api


    Utils::named_lock("db.intenal.clients", [&](){

        //check if cliente is already registred
        if (!db->exists("internal.clients.byId."+clientId))
        {
            //new client
            int clientsCount = db->get("internal.clients.list.count", 0).getInt();
            db->set("internal.clients.list.count", clientsCount+1);
            db->set("internal.clients.list."+to_string(clientsCount), clientId);
            db->set("internal.clients.byId."+clientId, clientsCount);


        }
        //update client keepAlive and apiId
        db->set("internal.clients.byId."+clientId+".apiId", api->getApiId());    
        this->updateLiveTime();
    });
}


Controller_ClientHelper::Controller_ClientHelper(StorageInterface *db, string clientId, ApiInterface* api): db(db), clientId(clientId), api (api) {

    this->initialize();
}

Controller_ClientHelper::Controller_ClientHelper(StorageInterface *db, string clientId, map<string, ApiInterface*> apis, Controller_ClientHelperOnError onError): db(db), clientId(clientId)
{
    auto apiId = db->get("internal.clients.byId."+clientId+".apiId", "").getString();    
    if (apis.count(apiId) > 0)
        this->api = apis[apiId];
    else
        onError(Controller_ClientHelperError::API_NOT_FOUND);
}

int64_t Controller_ClientHelper::getLastLiveTime()
{
    return db->get("internal.clients.byId."+clientId+".lastLiveTime", 0).getInt64();
}

int64_t Controller_ClientHelper::timeSinceLastLiveTime()
{
    return Utils::getCurrentTimeSeconds() - this->getLastLiveTime();
}

void Controller_ClientHelper::updateLiveTime()
{
    db->set("internal.clients.byId."+clientId+".lastLiveTime", Utils::getCurrentTimeSeconds());
}

bool Controller_ClientHelper::isConnected()
{
    auto ret = this->api->checkAlive(this->clientId) == API::ClientSendResult::LIVE;
    if (ret)
        this->updateLiveTime();

    return ret;
}

vector<string> Controller_ClientHelper::getObservingVars()
{
    vector<string> result;
    vector<string> tmp = db->getChilds("internal.clients.byId."+clientId+".observing");;
    for (auto &c : tmp)
        if (c.find(".count") == string::npos && c.find(".size") == string::npos)
            result.push_back(db->get(c, "").getString());

    return result;
}

API::ClientSendResult Controller_ClientHelper::notify(vector<tuple<string, DynamicVar>> varsAndValues)
{
    if (this->api->notifyClient(clientId, varsAndValues) == ClientSendResult::LIVE)
    {
        this->updateLiveTime();
        return API::ClientSendResult::LIVE;
    }
    return API::ClientSendResult::DISCONNECTED;

}

void Controller_ClientHelper::registerNewObservation(string varName)
{
    Utils::named_lock("db.intenal.clients", [&](){
        this->updateLiveTime();

        auto currentCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
        db->set("internal.clients.byId."+clientId+".observing.count", currentCount+1);

        db->set("internal.clients.byId."+clientId+".observing."+to_string(currentCount), varName);
    });
}


string Controller_ClientHelper::getClientId()
{
    return this->clientId;
}

void Controller_CLientHelper::removeClienteFromObservationSystem()
{
    
}