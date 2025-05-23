#include  "Controller_ClientHelper.h" 
 
void Controller_ClientHelper::initialize()
{
    //try find api


    Utils::named_lock("db.intenal.clients", [&](){

        //check if cliente is already registred
        if (!db->hasValue("internal.clients.byId."+clientId))
        {
            //new client
            int clientsCount = db->get("internal.clients.list.count", 0).getInt();
            db->set("internal.clients.list.count", clientsCount+1);
            db->set("internal.clients.list."+to_string(clientsCount), clientId);
            db->set("internal.clients.byId."+clientId, clientsCount);


        }

        //cout << "Storging API id "+api->getApiId() + " for the client "+clientId << endl;
        db->set("internal.clients.byId."+clientId+".apiId", api->getApiId());    
        //update client keepAlive and apiId
        this->updateLiveTime();
    }, 2000);
}


Controller_ClientHelper::Controller_ClientHelper(StorageInterface *db, string clientId, ApiInterface* api): db(db), clientId(clientId), api (api) {

    this->initialize();
}

Controller_ClientHelper::Controller_ClientHelper(StorageInterface *db, string clientId, map<string, ApiInterface*> apis, Controller_ClientHelperError &error): db(db), clientId(clientId)
{
    error = Controller_ClientHelperError::NO_ERROR;
    auto apiId = db->get("internal.clients.byId."+clientId+".apiId", "").getString();    
    if (apis.count(apiId) > 0)
        this->api = apis[apiId];
    else
        error = Controller_ClientHelperError::API_NOT_FOUND;
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

int Controller_ClientHelper::getObservingVarsCount()
{
    return db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
}

vector<string> Controller_ClientHelper::getObservingVars()
{
    vector<string> result;
    vector<string> tmp = db->getChilds("internal.clients.byId."+clientId+".observing");;
    for (auto &c : tmp)
        if (c.find("count") == string::npos && c.find("size") == string::npos)
        {
            auto currChildName = db->get("internal.clients.byId."+clientId+".observing."+c, "").getString();
            //remove the "vars." from begining o ght currChildName
            if (currChildName.size() > 5)
                currChildName = currChildName.substr(5);

            result.push_back(currChildName);
        }

    return result;
}

API::ClientSendResult Controller_ClientHelper::notify(vector<tuple<string, string, DynamicVar>> varsnamesMetadaAndValues)
{
    if (this->api->notifyClient(clientId, varsnamesMetadaAndValues) == ClientSendResult::LIVE)
    {
        this->updateLiveTime();
        return API::ClientSendResult::LIVE;
    }
    return API::ClientSendResult::DISCONNECTED;

}

void Controller_ClientHelper::registerNewObservation(string varName)
{
    varName = "vars."+varName;
    Utils::named_lock("db.intenal.clients", [&](){
        this->updateLiveTime();

        auto currentCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
        db->set("internal.clients.byId."+clientId+".observing.count", currentCount+1);
        db->set("internal.clients.byId."+clientId+".observing."+to_string(currentCount), varName);
    }, 2000);
}


string Controller_ClientHelper::getClientId()
{
    return this->clientId;
}




void Controller_ClientHelper::removeClientFromObservationSystem()
{
    //completelly remove the client infor from obsrevation system and from variables
    Utils::named_lock("db.intenal.clients", [&](){
        
        auto clientIndexOnInternalDBList = db->get("internal.clients.byId."+clientId, -1).getInt();

        if (clientIndexOnInternalDBList > -1)
        {

            //todo: DELETE ALSO FROM internal.clients.list 

            auto currentCount = db->get("internal.clients.list.count", 0).getInt();
            for (int c = clientIndexOnInternalDBList; c < currentCount-1; c++)
            {
                auto currentId = db->get("internal.clients.list."+to_string(c+1), "");
                db->set("internal.clients.list."+to_string(c), currentId);
            }

            currentCount--;
            db->deleteValue("internal.clients.list."+to_string(currentCount));
            db->set("internal.clients.list.count", currentCount);
        }
        db->deleteValue("internal.clients.byId."+clientId, true);

    }, 5000);

}

void Controller_ClientHelper::unregisterObservation(string varName)
{
    varName = "vars."+varName;
    Utils::named_lock("db.intenal.clients", [&](){
        this->updateLiveTime();
        
        auto currentCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();
        auto varIndex = findVarIndexOnObservingVars(varName);
        if (varIndex > -1)
        {
            for (int c = varIndex; c < currentCount-1; c++)
            {
                auto currVar = db->get("internal.clients.byId."+clientId+".observing."+to_string(c+1), "");
                db->set("internal.clients.byId."+clientId+".observing."+to_string(c), currVar);
            }

            db->deleteValue("internal.clients.byId."+clientId+".observing."+to_string(currentCount-1));

            db->set("internal.clients.byId."+clientId+".observing.count", currentCount-1);
        }
        
    }, 5000);
}

int Controller_ClientHelper::findVarIndexOnObservingVars(string varName)
{
    auto currentCount = db->get("internal.clients.byId."+clientId+".observing.count", 0).getInt();

    for (int c = 0; c < currentCount; c++)
    {
        auto currVar = db->get("internal.clients.byId."+clientId+".observing."+to_string(c), "").getString();
        if (currVar == varName)
            return c;
    }

    return -1;
}
