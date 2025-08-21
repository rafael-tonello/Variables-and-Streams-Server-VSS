#include "Controller.h"

using namespace Controller;

TheController::TheController(DependencyInjectionManager* dim, string systemVersion): systemVersion(systemVersion)
{
    
    this->log = dim->get<ILogger>();

    this->confs = dim->get<Confs>();

    //Moved to the main.cpp
    //this->confs->createAlias("maxTimeWaitingClient_seconds").addForAnyProvider({"maxTimeWaitingClients_seconds", "--maxTimeWaitingForClients", "VSS_MAX_TIME_WAITING_CLIENTS"});

    if (dim->contains<ThreadPool>())
    {
        this->tasker = dim->get<ThreadPool>();

    }
    else
    {
        log->info("TheController", "No thread pool found. creating a new one");
        this->tasker = new ThreadPool(4);
    }

    if (this->confs == NULL)
        log->critical("TheController", "configuration system can't be found in the dependency injection manager");


    db = dim->get<StorageInterface>();

    this->confs->listenA("maxTimeWaitingClient_seconds", [&](DynamicVar newValue){
        this->maxTimeWaitingClient_seconds = newValue.getInt64();
    }, true, maxTimeWaitingClient_seconds);

    srand(Utils::getCurrentTimeMilliseconds());
}

TheController::~TheController()
{
    
}

//creates or change a variable
future<Errors::Error> TheController::setVar(string name, DynamicVar value)
{
    return tasker->enqueue([this](string namep, DynamicVar valuep)
    {
        namep = Utils::getOnly(namep, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._/\\,.-");
        
        log->debug("Entered in setVar task");
        //check if name isn't a internal flag var

        if (namep.find('_') == 0 || namep.find("._") != string::npos)
        {
            log->warning("TheController", Errors::Error_VariablesStartedWithUnderscornAreJustForInternal);
            return Errors::Error_VariablesStartedWithUnderscornAreJustForInternal;
        }

        Controller_VarHelper varHelper(log, this->db, namep);
        //checks if the variabel is locked
        if (varHelper.isLocked())
        {
            Errors::Error errMessage = Utils::sr(Errors::Error_TheVariable_name_IsLocketAndCantBeChangedBySetVar, "vName", namep);
            log->warning("TheController", errMessage);
            return errMessage;
        }

        auto ret = varHelper.setValue(valuep);
        this->notifyVarModification(namep, valuep);

        log->debug("Will exit from setVar task");
        return ret;
    }, name, value);

}

future<Errors::Error> TheController::lockVar(string varName, uint maxTimeOut_ms)
{
    return tasker->enqueue([=, this](string varNamep){
        Controller_VarHelper varHelper(log, db, varNamep);
        return varHelper.lock(maxTimeOut_ms);

    }, varName);
}

future<Errors::Error> TheController::unlockVar(string varName)
{
    return tasker->enqueue([&](string varNamep){
        Controller_VarHelper varHelper(log, db, varNamep);
        varHelper.unlock();

        return Errors::NoError;
    }, varName);
}

bool TheController::isVarLocked(string varName)
{
    Controller_VarHelper varHelper(log, db, varName);
    return varHelper.isLocked();
}

//start to observate a variable
void TheController::observeVar(string varName, string clientId, string customIdsAndMetainfo, ApiInterface* api)
{
    //NOTE: enclosure the code ins a ThreadPool tasker
    log->info("TheController", {"::observeVar called. varName: '",varName,"', clientId:: '",clientId,"', api(id): '",api->getApiId(),"'"});

    
    Controller_VarHelper varHelper(log, db, varName);
    if (!varHelper.isObserving(clientId, customIdsAndMetainfo))
    {
        Controller_ClientHelper client(this->db, clientId, api);

        varHelper.addObserver(clientId, customIdsAndMetainfo);
        client.registerNewObservation(varName);

        //upate client about the var
        auto getVarResult = this->getVar(varName, "").get().result;

        vector<tuple<string, string, DynamicVar>> finalResult;

        for (auto &c: getVarResult)
            finalResult.push_back(std::make_tuple(std::get<0>(c), customIdsAndMetainfo, std::get<1>(c)));
        

        if (client.notify(finalResult) != API::ClientSendResult::LIVE)
            this->checkClientLiveTime(client);
    };
}

void TheController::notifyVarModification(string varName, DynamicVar value)
{
    tasker->enqueue([varName, value, this](){
        Controller_VarHelper vh(log, db, varName);

        notifyClientsAboutVarChange(vh.getObservations(), varName, value);

        notifyParentGenericObservers(varName, varName, value);

        /*done: If the current variable contains the child '*', should it be notified?
            when i observe the variable "a.b.c.*" i want to observate also a.b.c or only their childs.*/
        Controller_VarHelper childWildcard(log, db, varName + ".*");
        notifyClientsAboutVarChange(childWildcard.getObservations(), varName, value);
    });
    
}

void TheController::notifyParentGenericObservers(string varName, string changedVarName, DynamicVar value)
{       
    if (varName.find(".") != string::npos)
    {
        string parentName = varName.substr(0, varName.find_last_of("."));
        Controller_VarHelper varHelper(log, db, parentName + ".*");
        notifyClientsAboutVarChange(varHelper.getObservations(), changedVarName, value);
        notifyParentGenericObservers(parentName, changedVarName, value);
    }
}

void TheController::notifyClientsAboutVarChange(vector<tuple<string, string>> clientIdsAndMetadata, string changedVarName, DynamicVar value)
{
    //notify firsts added first
    //for (tuple<string, string> &clientIdAndMetadata: clientIdsAndMetadata)

    //notify last added first
    for (int c = clientIdsAndMetadata.size()-1; c >= 0; c--)
    {
        auto tmp = clientIdsAndMetadata[c];
        tasker->enqueue([&](string changedVarNamep, tuple<string, string> clientIdAndMetadatap, DynamicVar valuep){
            auto clientIdp = std::get<0>(clientIdAndMetadatap);
            auto customIdsAndMetainfo = std::get<1>(clientIdAndMetadatap);
            Controller_ClientHelperError resultError;
            Controller_ClientHelper ch(db, clientIdp, this->apis, resultError);

            if (resultError == Controller_ClientHelperError::NO_ERROR)
            {
                if (ch.notify( { std::make_tuple(changedVarNamep, customIdsAndMetainfo, valuep) } ) != API::ClientSendResult::LIVE)
                    this->checkClientLiveTime(ch);
            }
            else if (resultError == Controller_ClientHelperError::API_NOT_FOUND)
            {
                log->error("TheController", Utils::srm("Client notification failute due 'responsible API not found (CliId = '?', Api = '?').", {
                    clientIdp,
                    db->get("internal.clients.byId."+clientIdp+".apiId", "").getString(),
                }));
                //try  to remove invalid clients (but this cannot happenens :( ))
                deleteClient(ch);
            }
            else
                log->error("TheController", "Client notification failute due an unknown error.");
        }, changedVarName, tmp, value);
    }
}

//stop observate variable
void TheController::stopObservingVar(string varName, string clientId, string customIdsAndMetainfo, ApiInterface* api)
{
    Controller_VarHelper varHelper(log, db, varName);
    Controller_ClientHelper clienthelper(db, clientId, api);

    varHelper.removeObserving(clientId, customIdsAndMetainfo);
    clienthelper.unregisterObservation(varName);
}

void TheController::apiStarted(ApiInterface *api)
{
    this->apis[api->getApiId()] = api;
}

string TheController::_createUniqueId()
{
    string tmp = to_string(Utils::getCurrentTimeMilliseconds());
    tmp += to_string(rand()) + to_string(rand());
    return Utils::StringToHex(tmp);
}

future<GetVarResult> TheController::getVar(string name, DynamicVar defaultValue)
{
    return tasker->enqueue([&](string namep, DynamicVar defaultValuep){

        if (namep == "")
        {
            log->warning("TheController", Errors::Error_TheVariableNameCannotBeEmpty);
            return GetVarResult({}, Errors::Error_TheVariableNameCannotBeEmpty);
        }

        function <vector<tuple<string, DynamicVar>>(string namep, bool childsToo)> readFromDb;
        readFromDb = [&](string nname, bool childsToo)        
        {
            vector<tuple<string, DynamicVar>> result;
            Controller_VarHelper varHelper(log, db, nname);

            if (varHelper.valueIsSetInTheDB())
                result.push_back(make_tuple( nname,  varHelper.getValue()));
            
            if (childsToo)
            {
                nname = nname == "" ? nname : nname + ".";
                auto childs = varHelper.getChildsNames();
                for (auto curr: childs)
                {
                    auto tmp = readFromDb(nname + curr, childsToo);                    
                    result.insert(result.begin(), tmp.begin(), tmp.end());
                }
            }

            return result;
        };

        //if variable ends with *, determine just their name
        bool childsToo = false;
        if (namep.find("*") != string::npos)
        {
            if (namep.find("*") == namep.size()-1)
            {
                childsToo = true;
                auto p = namep.find(".");
                if (p != string::npos && p == namep.size()-2)
                    namep = namep.substr(0, namep.size()-2);
                else
                    namep = namep.substr(0, namep.size()-1);
            }
            else
            {
                log->error("TheController", Errors::Error_WildCardCabBeUsedOnlyAtEndOfVarNameForVarGetting);
                return GetVarResult(Errors::Error_WildCardCabBeUsedOnlyAtEndOfVarNameForVarGetting, {});
            }
        }

        //load the valures
        auto values = readFromDb(namep, childsToo);
        

        //if nothing was found, add the default value to the result
        if (values.size() == 0)
            values.push_back(make_tuple(namep, defaultValuep));

        //return the values
        return GetVarResult(Errors::NoError, values);
    }, name, defaultValue);

}

future<Errors::Error> TheController::delVar(string varname)
{
    return tasker->enqueue([&](string varnamep)
    {
        Controller_VarHelper varHelper(log, db, varnamep);
        if (varHelper.deleteValueFromDB())
            return Errors::NoError;
        else
            return Errors::Error("Error deleting variable '"+varname+"'. Maybe the variable doesn't exist");
    }, varname);
}

future<Errors::ResultWithStatus<vector<string>>> TheController::getChildsOfVar(string parentName)
{
    return tasker->enqueue([&](string parentNamep)
    {
        if (parentNamep.find('*') != string::npos)
        {
            log->error("TheController", Errors::Error_WildcardCanotBeUsesForGetVarChilds);
            return Errors::ResultWithStatus<vector<string>>(Errors::Error_WildcardCanotBeUsesForGetVarChilds, {});
        }

        Controller_VarHelper varHelper(log, db, parentNamep);
        auto result = varHelper.getChildsNames();
        return Errors::ResultWithStatus<vector<string>>(Errors::NoError, result);
    }, parentName);

}

string TheController::clientConnected(string clientId, ApiInterface* api, int &observingVarsCount)
{
    if (clientId == "")
        clientId = _createUniqueId();

    Controller_ClientHelper client = Controller_ClientHelper(db, clientId, api);
    observingVarsCount = client.getObservingVarsCount();

    this->log->debug("The client '"+clientId+"' has "+to_string(observingVarsCount)+" observing variables. Updating client about the current values.");

    updateClientAboutObservatingVars(client);

    return clientId;
}

void TheController::updateClientAboutObservatingVars(Controller_ClientHelper controller_ClientHelper)
{

    tasker->enqueue([&](Controller_ClientHelper controller_ClientHelperP){
        //For all variabels observed by this client, sent the current value
        bool mustCheckClientLiveTime = false;
        auto observingVars = controller_ClientHelperP.getObservingVars();

        for (auto &currVarToSearch : observingVars)
        {
            //vector<tuple<varname, customIdsAndMetainfo, value>>
            vector<tuple<string, string, DynamicVar>> result;

            auto varSearchResult = this->getVar(currVarToSearch, "").get().result;
            for (auto &c: varSearchResult)
            {
                Controller_VarHelper currVar(log, this->db, std::get<0>(c));

                auto customIdsAndMetainfos = currVar.getMetadatasOfAClient(controller_ClientHelperP.getClientId());
                for (auto &currMetadata: customIdsAndMetainfos)
                    result.push_back({
                        std::get<0>(c), 
                        currMetadata,
                        std::get<1>(c)
                    });
            }

            this->log->debug("Upading client about the variable '"+currVarToSearch+"'");
            if (controller_ClientHelperP.notify(result) != API::ClientSendResult::LIVE)
            {
                mustCheckClientLiveTime = true;
                break;
            }
        }

        if (mustCheckClientLiveTime)
            this->checkClientLiveTime(controller_ClientHelperP);
    }, controller_ClientHelper);
}

void TheController::checkClientLiveTime(Controller_ClientHelper client)
{
    if (!client.isConnected())
    {
        if (client.timeSinceLastLiveTime() >= maxTimeWaitingClient_seconds)
        {
            deleteClient(client);
        }
    }
}

void TheController::deleteClient(Controller_ClientHelper client)
{
    auto vars = client.getObservingVars();
    client.removeClientFromObservationSystem();
    for (auto &currVar: vars)
    {
        Controller_VarHelper varHelper(log, db, currVar);
        varHelper.removeCliObservings(client.getClientId());
    }
}

string TheController::getSystemVersion()
{
    return systemVersion;
}