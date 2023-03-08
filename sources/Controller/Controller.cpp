#include "Controller.h"

using namespace Controller;

TheController::TheController(DependencyInjectionManager* dim)
{
    
    this->log = dim->get<ILogger>();

    this->confs = dim->get<Config>();

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

    this->confs->observate("maxTimeWaitingClient_seconds", [&](DynamicVar newValue){
        this->maxTimeWaitingClient_seconds = newValue.getInt64();
    }, maxTimeWaitingClient_seconds);

    srand(Utils::getCurrentTimeMilliseconds());
}

TheController::~TheController()
{
    
}

//creates or change a variable
future<Errors::Error> TheController::setVar(string name, DynamicVar value)
{
    Controller_VarHelper varHelper(log, this->db, name);
    //check if name isn't a internal flag var

    if (name.find('_') == 0 || name.find("._") != string::npos)
    {
        log->warning("TheController", Errors::Error_VariablesStartedWithUnderscornAreJustForInternal);
        promise<Errors::Error> p;
        p.set_value(Errors::Error_VariablesStartedWithUnderscornAreJustForInternal);
        return p.get_future();
    }

    //checks if the variabel is locked
    if (varHelper.isLocked())
    {
        Errors::Error errMessage = Utils::sr(Errors::Error_TheVariable_name_IsLocketAndCantBeChangedBySetVar, "vName", name);
        log->warning("TheController", errMessage);
        promise<Errors::Error> p;
        p.set_value(errMessage);
        return p.get_future();
    }

    return tasker->enqueue([this](string namep, Controller_VarHelper varhelperp, DynamicVar valuep)
    {
        auto ret = varhelperp.setValue(valuep);
        this->notifyVarModification(namep, valuep);
        return ret;
    }, name, varHelper, value);

}
  
future<Errors::Error> TheController::lockVar(string varName)
{
    return tasker->enqueue([this](string varNamep){
        Controller_VarHelper varHelper(log, db, varNamep);
        varHelper.lock();

        return Errors::NoError;
    }, varName);
}

future<Errors::Error> TheController::unlockVar(string varName)
{
    return tasker->enqueue([this](string varNamep){
        Controller_VarHelper varHelper(log, db, varNamep);
        varHelper.unlock();

        return Errors::NoError;
    }, varName);
}

//start to observate a variable
void TheController::observeVar(string varName, string clientId, string customMetadata, ApiInterface* api)
{
    //NOTE: enclosure the code ins a ThreadPool tasker
    log->info("TheController", {"::observeVar called. varName: '",varName,"', clientId:: '",clientId,"', api(id): '",api->getApiId(),"'"});

    
    Controller_VarHelper varHelper(log, db, varName);
    if (!varHelper.isClientObserving(clientId))
    {
        Controller_ClientHelper client(this->db, clientId, api);

        varHelper.addClientToObservers(clientId, customMetadata);
        client.registerNewObservation(varName);

        //upate client about the var
        auto getVarResult = this->getVar(varName, "").get().result;

        vector<tuple<string, string, DynamicVar>> finalResult;

        for (auto &c: getVarResult)
            finalResult.push_back(std::make_tuple(std::get<0>(c), customMetadata, std::get<1>(c)));
        

        if (client.notify(finalResult) != API::ClientSendResult::LIVE)
            this->checkClientLiveTime(client);
    };
}

void TheController::notifyVarModification(string varName, DynamicVar value)
{
    Controller_VarHelper vh(log, db, varName);
    notifyClientsAboutVarChange(vh.getObserversClientIdsAndMetadta(), varName, value);

    //todo: scrolls through parent vars lookin for '*' special child
    notifyParentGenericObservers(varName, varName, value);

    /*done: If the current variable contains the child '*', should it be notified?
        when i observe the variable "a.b.c.*" i want to observate also a.b.c or only their childs.*/
   Controller_VarHelper childWildcard(log, db, varName + ".*");
   notifyClientsAboutVarChange(childWildcard.getObserversClientIdsAndMetadta(), varName, value);
    
}

void TheController::notifyParentGenericObservers(string varName, string changedVarName, DynamicVar value)
{       
    if (varName.find(".") != string::npos)
    {
        string parentName = varName.substr(0, varName.find_last_of("."));
        Controller_VarHelper varHelper(log, db, parentName + ".*");
        notifyClientsAboutVarChange(varHelper.getObserversClientIdsAndMetadta(), changedVarName, value);
        notifyParentGenericObservers(parentName, changedVarName, value);
    }
}

void TheController::notifyClientsAboutVarChange(vector<tuple<string, string>> clientIdsAndMetadata, string changedVarName, DynamicVar value)
{
    for (tuple<string, string> &clientIdAndMetadata: clientIdsAndMetadata)
    {
        tasker->enqueue([&](string changedVarNamep, tuple<string, string> clientIdAndMetadatap, DynamicVar valuep){
            auto clientIdp = std::get<0>(clientIdAndMetadatap);
            auto metadata = std::get<1>(clientIdAndMetadatap);
            Controller_ClientHelperError resultError;
            Controller_ClientHelper ch(db, clientIdp, this->apis, resultError);

            if (resultError == Controller_ClientHelperError::NO_ERROR)
            {
                if (ch.notify( { std::make_tuple(changedVarNamep, metadata, valuep) } ) != API::ClientSendResult::LIVE)
                    this->checkClientLiveTime(ch);
            }
            else if (resultError == Controller_ClientHelperError::API_NOT_FOUND)
                log->error("TheController", "Client notification failute due 'responsible API not found.");
            else
                log->error("TheController", "Client notification failute due an unknown error.");
        }, changedVarName, clientIdAndMetadata, value);
    }
}

//stop observate variable
void TheController::stopObservingVar(string varName, string clientId, ApiInterface* api)
{
    Controller_VarHelper varHelper(log, db, varName);
    Controller_ClientHelper clienthelper(db, clientId, api);

    varHelper.removeClientFromObservers(clientId);
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
    return tasker->enqueue([this](string namep, DynamicVar defaultValuep){

        if (namep == "")
        {
            log->warning("TheController", Errors::Error_TheVariableNameCannotBeEmpty);
            return GetVarResult(Errors::Error_TheVariableNameCannotBeEmpty, {});
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
    return tasker->enqueue([this](string varnamep)
    {
        Controller_VarHelper varHelper(log, db, varnamep);
        varHelper.deleteValueFromDB();
        return Errors::NoError;
    }, varname);
}

future<vector<string>> TheController::getChildsOfVar(string parentName)
{
    return tasker->enqueue([this](string parentNamep)
    {
        Controller_VarHelper varHelper(log, db, parentNamep);
        return varHelper.getChildsNames();
    }, parentName);

}

string TheController::clientConnected(string clientId, ApiInterface* api, int &observingVarsCount)
{
    if (clientId == "")
        clientId = _createUniqueId();

    Controller_ClientHelper client = Controller_ClientHelper(db, clientId, api);
    observingVarsCount = client.getObservingVarsCount();

    updateClientAboutObservatingVars(client);

    return clientId;
}

void TheController::updateClientAboutObservatingVars(Controller_ClientHelper controller_ClientHelper)
{

    tasker->enqueue([&](Controller_ClientHelper controller_ClientHelperP){
        //For all variabels observed by this client, sent the current value
        bool mustCheckClientLiveTime = false;
        auto observingVars = controller_ClientHelperP.getObservingVars();

        for (auto &curr : observingVars)
        {
            auto currVarsAndValues = this->getVar(curr, "").get().result;

            vector<tuple<string, string, DynamicVar>> finalResult;
            Controller_VarHelper tmpVar(log, this->db, curr);

            for (auto &c: currVarsAndValues)
                finalResult.push_back(std::make_tuple(std::get<0>(c), tmpVar.getMetadataForClient(controller_ClientHelper.getClientId()), std::get<1>(c)));

            if (controller_ClientHelperP.notify(finalResult) != API::ClientSendResult::LIVE)
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
        if (!client.timeSinceLastLiveTime() >= maxTimeWaitingClient_seconds)
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
        varHelper.removeClientFromObservers(client.getClientId());
    }
}