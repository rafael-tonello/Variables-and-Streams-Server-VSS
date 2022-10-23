#include "Controller.h"

using namespace Controller;

TheController::TheController(DependencyInjectionManager* dim)
{
    
    this->log = dim->get<ILogger>();
    this->confs = dim->get<Config>();
    this->bus = dim->get<MessageBus<JsonMaker::JSON>>();

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
        log->error("TheController", "configuration system can't be found in the dependency injection manager");


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
future<void> TheController::setVar(string name, DynamicVar value)
{
    #pragma region hooking 
    {
        JSON args;
        args.setStrign("name", name);
        args.setString("value", value.getString());
        auto busResult = bus->post("hooking.TheController.setVar", args);
        if (busResult.size() >0)
        {
            if (!busResult[0].getBoolean("continue", true))
            {
                log->info2("'setVar' execution was aborted by an interceptor (via MessageBus)");
                return tasker.enqueue([](){});
            }
            name = busResult[0].getString("name", name);
            value = DynamicVar(busResult[0].getString("value", name));
        }
    }
    #pragma endregion

    Controller_VarHelper varHelper(log, this->db, name);
    //check if name isn't a internal flag var

    if (name.find('_') == 0 || name.find("._") != string::npos)
    {
        log->warning("TheController", "Variabls started with underscorn (_) are just for internal flags and can't be setted by clients");
        promise<void> p;
        p.set_value();
        return p.get_future();
    }

    //checks if the variabel is locked
    if (varHelper.isLocked())
    {
        log->warning("TheController", "The variable '"+ name + "' is locked and can't be changed by setVar");
        promise<void> p;
        p.set_value();
        return p.get_future();
    }

    return tasker->enqueue([this](string namep, Controller_VarHelper varhelperp, DynamicVar valuep)
    {
        varhelperp.setValue(valuep);
        this->notifyVarModification(namep, valuep);
    }, name, varHelper, value);

}

future<void> TheController::lockVar(string varName)
{
    #pragma region hooking 
    {
        JSON args;
        args.setString("varName", varName);
        if (auto busResult = bus->post("hooking.TheController.lockVar", args); busResult.size() >0)
        {
            if (!busResult[0].getBoolean("continue", true))
            {
                log->info2("'lockVar' execution was aborted by an interceptor (via MessageBus)");
                return tasker.enqueue([](){});
            }
            varName = busResult[0].getString("varName", varName);
        }
    }
    #pragma endregion

    return tasker->enqueue([this](string varNamep){
        Controller_VarHelper varHelper(log, db, varNamep);
        varHelper.lock();
    }, varName);
}

future<void> TheController::unlockVar(string varName)
{
    #pragma region hooking 
    {
        JSON args;
        args.setString("varName", varName);
        if (auto busResult = bus->post("hooking.TheController.unlockVar", args); busResult.size() >0)
        {
            if (!busResult[0].getBoolean("continue", true))
            {
                log->info2("'unlockVar' execution was aborted by an interceptor (via MessageBus)");
                return tasker.enqueue([](){});
            }
            varName = busResult[0].getString("varName", varName);
        }
    }
    #pragma endregion

    return tasker->enqueue([this](string varNamep){
        Controller_VarHelper varHelper(log, db, varNamep);
        varHelper.unlock();
    }, varName);
}

//start to observate a variable
void TheController::observeVar(string varName, string clientId, ApiInterface* api)
{
    #pragma region hooking 
    {
        JSON args;
        args.setString("varName", varName);
        args.setString("clientId", clientId);
        args.setString("api", to_string((uint64_t)((void*)api))+"}");
        auto busResult = bus->post("hooking.TheController.observeVar", args);
        if (busResult.size() >0)
        {
            if (!busResult[0].getBoolean("continue", true))
            {
                log->info2("'observeVar' execution was aborted by an interceptor (via MessageBus)");
                return;
            }
            varName = busResult[0].getString("varName", name);
            clientId = DynamicVar(busResult[0].getString("clientId", name));
            api = (ApiInterface*)((uint64_t)std::stoull(busResult[0].getString("api", to_string((uint64_t)((void*)api)))));
        }
    }
    #pragma endregion

    //NOTE: enclosure the code ins a ThreadPool tasker
    log->info("TheController", {"::observeVar called. varName: '",varName,"', clientId:: '",clientId,"', api(id): '",api->getApiId(),"'"});

    
    Controller_VarHelper varHelper(log, db, varName);
    if (!varHelper.isClientObserving(clientId))
    {
        Controller_ClientHelper client(this->db, clientId, api);

        varHelper.addClientToObservers(clientId);
        client.registerNewObservation(varName);

        //upate client about the var
        auto varsResult = this->getVar(varName, "").get();
        

        if (client.notify(varsResult) != API::ClientSendResult::LIVE)
            this->checkClientLiveTime(client);
    };
}

void TheController::notifyVarModification(string varName, DynamicVar value)
{
    Controller_VarHelper vh(log, db, varName);
    notifyClientsAboutVarChange(vh.getObserversClientIds(), varName, value);

    //todo: scrolls through parent vars lookin for '*' special child
    notifyParentGenericObservers(varName, varName, value);

    /*done: If the current variable contains the child '*', should it be notified?
        when i observe the variable "a.b.c.*" i want to observate also a.b.c or only their childs.*/
   Controller_VarHelper childWildcard(log, db, varName + ".*");
   notifyClientsAboutVarChange(childWildcard.getObserversClientIds(), varName, value);
    
}

void TheController::notifyParentGenericObservers(string varName, string changedVarName, DynamicVar value)
{       
    if (varName.find(".") != string::npos)
    {
        string parentName = varName.substr(0, varName.find_last_of("."));
        Controller_VarHelper varHelper(log, db, parentName + ".*");
        notifyClientsAboutVarChange(varHelper.getObserversClientIds(), changedVarName, value);
        notifyParentGenericObservers(parentName, changedVarName, value);
    }
}

void TheController::notifyClientsAboutVarChange(vector<string> clients, string changedVarName, DynamicVar value)
{
    for (string &clientId: clients)
    {
        tasker->enqueue([&](string changedVarNamep, string clientIdp, DynamicVar valuep){
            
            Controller_ClientHelperError resultError;
            Controller_ClientHelper ch(db, clientIdp, this->apis, resultError);

            if (resultError == Controller_ClientHelperError::NO_ERROR)
            {
                if (ch.notify( { std::make_tuple(changedVarNamep, valuep) } ) != API::ClientSendResult::LIVE)
                    this->checkClientLiveTime(ch);
            }
            else if (resultError == Controller_ClientHelperError::API_NOT_FOUND)
                log->error("TheController", "Client notification failute due 'responsible API not found.");
            else
                log->error("TheController", "Client notification failute due an unknown error.");
        }, changedVarName, clientId, value);
    }
}

//stop observate variable
void TheController::stopObservingVar(string varName, string clientId, ApiInterface* api)
{
    #pragma region hooking 
    {
        JSON args;
        args.setString("varName", varName);
        args.setString("clientId", clientId);
        args.setString("api", to_string((uint64_t)((void*)api))+"}");
        auto busResult = bus->post("hooking.TheController.stopObservingVar", args);
        if (busResult.size() >0)
        {
            if (!busResult[0].getBoolean("continue", true))
            {
                log->info2("'stopObservingVar' execution was aborted by an interceptor (via MessageBus)");
                return;
            }
            varName = busResult[0].getString("varName", name);
            clientId = DynamicVar(busResult[0].getString("clientId", name));
            api = (ApiInterface*)((uint64_t)std::stoull(busResult[0].getString("api", to_string((uint64_t)((void*)api)))));
        }
    }
    #pragma endregion

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

future<vector<tuple<string, DynamicVar>>> TheController::getVar(string name, DynamicVar defaultValue)
{
    return tasker->enqueue([this](string namep, DynamicVar defaultValuep){
        #pragma region hooking 
        {
            JSON args;
            args.setString("name", name);
            args.setString("defaultValue", defaultValue);
            auto busResult = bus->post("hooking.TheController.getVar", args);
            if (busResult.size() >0)
            {
                if (!busResult[0].getBoolean("continue", true))
                {
                    log->info2("'getVar' execution was aborted by an interceptor (via MessageBus)");
                    vector<tuple<string, DynamicVar>> result;
                    for (auto &c: busResult[0].getChildsNames("result"))
                    {
                        resutl.push_back({
                            busResult[0].getString("result."+c+".key", "");
                            busResult[0].getString("result."+c+".value", "");
                        });

                    }

                    return result;
                }
                name = busResult[0].getString("name", name);
                defaultValue = DynamicVar(busResult[0].getString("defaultValue", name));
                api = (ApiInterface*)((uint64_t)std::stoull(busResult[0].getString("api", to_string((uint64_t)((void*)api)))));
            }
        }
        #pragma endregion


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
                log->error("TheController", "wildcard char (*) can be used only at last of varname in the getVar function");
                //vector<tuple<string, DynamicVar>> emptyResult = { std::make_tuple(namep, defaultValuep ) };
                vector<tuple<string, DynamicVar>> emptyResult = { };
                return emptyResult;
            }
        }

        //load the valures
        auto values = readFromDb(namep, childsToo);
        

        //if nothing was found, add the default value to the result
        if (values.size() == 0)
            values.push_back(make_tuple(namep, defaultValuep));

        //return the values

        return values;
    }, name, defaultValue);

}

future<void> TheController::delVar(string varname)
{
    #pragma region hooking 
    {
        JSON args;
        args.setString("varname", varname);
        if (auto busResult = bus->post("hooking.TheController.delVar", args); busResult.size() >0)
        {
            if (!busResult[0].getBoolean("continue", true))
            {
                log->info2("'delVar' execution was aborted by an interceptor (via MessageBus)");
                return tasker.enqueue([](){});
            }
            varname = busResult[0].getString("varname", varname);
        }
    }
    #pragma endregion


    return tasker->enqueue([this](string varnamep)
    {
        Controller_VarHelper varHelper(log, db, varnamep);
        varHelper.deleteValueFromDB();
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

string TheController::clientConnected(string clientId, ApiInterface* api)
{
    if (clientId == "")
        clientId = _createUniqueId();

    Controller_ClientHelper client = Controller_ClientHelper(db, clientId, api);

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
            auto currVarsAndValues = this->getVar(curr, "").get();

            if (controller_ClientHelperP.notify(currVarsAndValues) != API::ClientSendResult::LIVE)
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