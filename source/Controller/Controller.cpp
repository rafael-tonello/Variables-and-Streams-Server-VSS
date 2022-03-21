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
        log->error("TheController", "configuration system can't be found in the dependency injection manager");


    db = dim->get<StorageInterface>();

    this->confs->observate("maxTimeWaitingClient", [&](DynamicVar newValue){
        this->maxTimeWaitingClient = newValue.getInt64();
    }, maxTimeWaitingClient);

    srand(Utils::getCurrentTimeMilliseconds());
}

TheController::~TheController(){
    if (db != NULL) delete db;
}

//creates or change a variable
future<void> TheController::setVar(string name, DynamicVar value)
{
    name = "vars."+name;
    //check if name isn't a internal flag var
    if (name.find('_') == 0 || name.find("._") != string::npos)
    {
        log->warning("TheController", "variabls started with underscorn (_) are just for internal flags and can't be setted by clients");
        promise<void> p;
        p.set_value();
        return p.get_future();
    }

    //checks if the variabel is locked
    if (this->getVarInternalFlag(name, "_lock", 0).getInt() == 1)
    {
        log->warning("TheController", "The variable '"+ name + "' is locked and can't be changed by setVar");
        promise<void> p;
        p.set_value();
        return p.get_future();
    }

    return this->internalSetVar(name, value);

}

future<void> TheController::internalSetVar(string name, DynamicVar value)
{
    return tasker->enqueue([&](){

        vector<future<void>> pendingTasks;

        if (name.find('*') != string::npos)
        {
            #ifdef __TESTING__
                Tester::global_test_result = "Invalid setVar parameter. A variable with '*' can't be setted";
            #endif
            log->warning("TheController", "Invalid setVar parameter. A variable with '*' can't be setted");
            //throw "Invalid setVar parameter. A variable with '*' can't be setted";
            return;
        }


        //set the variable
        db->set(name, value.getString());

        notifyVarModification(name, value);
    });
}

future<void> TheController::lockVar(string varName)
{
    

    return tasker->enqueue([&](string vName){
        string finalVName = "vars."+varName + "._lock";
        //ir variable if currently locked, add an observer to it "._lock" property and wait the change of this to 0

        function<bool()> tryLockFunc = [&]()
        {
            //try lock var
            bool lockSucess = false;
            Utils::named_lock("varLockerSystem", [&]{
                if (db->get(finalVName, "0").getString() == "0")
                {
                    //set the var property '._lock' to 1 (use setVar to change the ._lock)
                    db->set(finalVName, "1");
                    lockSucess = db->get(finalVName, "0").getString() == "1";
                }
            });

            return lockSucess;
        };

        //await var be 0 to try another lock
        bool lockedWithSucess = false;
        while (!lockedWithSucess)
        {
            lockedWithSucess = tryLockFunc();

            if (!lockedWithSucess)
                usleep(1000 + rand() % 9000);
        }

        return;


    }, varName);


    //set var can't change a lokced var

}

future<void> TheController::unlockVar(string varName)
{
    return tasker->enqueue([&](){
        varLockerMutex.lock();
        string finalVName = "vars."+varName + "._lock";
        db->set(finalVName, "0");
        varLockerMutex.unlock();
    });

}

//start to observate a variable
void TheController::observeVar(string varName, string clientId, ApiInterface* api)
{
    varName = "vars."+varName;

    Utils::named_lock("vars."+varName + ".observationLock", [&](){

        Controller_ClientHelper client(this->db, clientId, api);
        if (!db->exists(varName + "._observers.byId."+clientId))
        {
            //Utils::named_lock("observingSystem"+varName, [&]() //more memory, lock by each variable
            Utils::named_lock("observingSystem", [&]() //less memory, lock all variables
            {
                int actualVar_observersCount = db->get(varName + "._observers.list.count", 0).getInt();
                db->set(varName + "._observers.list.count", actualVar_observersCount);

                db->set(varName + "._observers.list."+to_string(actualVar_observersCount), clientId);

                db->set(varName + "._observers.byId."+clientId, actualVar_observersCount);

                client.registerNewObservation(varName);
            });
        }
    });
    
}

void TheController::notifyVarModification(string name, DynamicVar value)
{
    /*string varName = "vars."+varName;

    int actualVar_observersCount = db->get(varName + "._observers.list.count", 0).getInt();
    for (int c =0; c < actualVar_observersCount; c++)
    {
        tasker->enqueue([&](string clientId){

        }, db->get(varName + "._observers.list."+to_string(c), "");
    }*/
}

//stop observate variable
void TheController::stopObservingVar(string clientId, string varName)
{
    varName = "vars."+varName;

    Utils::named_lock("vars."+varName + ".observationLock", [&](){

        int actualVar_observersCount = db->get(varName + "._observers.list.count", 0).getInt();

        for (int c = actualVar_observersCount-1; c>=0; c--)
        {
            if (db->get(varName + "._observers.list."+to_string(c), "").getString() == clientId)
            {
                for (int c2 = c; c2> < actualVar_observersCount; c2++)
                {
                    auto currId = db->get(varName + "._observers.list."+to_string(c2+1), "").getString();
                    db->set(varName+"._observers.list."+to_string(c), currId);

                    db->set(varName + "._observers.byId."+currId, c2);

                }

                //remove the last item from the _observers.list
                db->del(varName + "._observers.list."+to_string(actualVar_observersCount-1));
                actualVar_observersCount--;

                //remove the item from _observers.byId
                db->del(varName + "._observers.byId"+clientId);
            }
        }
    });
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
    name = "vars."+name;
    return tasker->enqueue([name, defaultValue, this](){

        function <vector<tuple<string, DynamicVar>>(string name, bool childsToo)> readFromDb;
        readFromDb = [&](string nname, bool childsToo)        
        {
            vector<tuple<string, DynamicVar>> result;    
            string value = db->get(nname, "___invalid____").getString();
            if (value != "___invalid___")
                result.push_back(make_tuple( nname,  value));
            
            if (childsToo)
            {
                auto childs = db->getChilds(name);
                for (auto curr: childs)
                {
                    auto tmp = readFromDb(curr, childsToo);                    
                    result.insert(result.begin(), tmp.begin(), tmp.end());
                }
            }

            return result;
        };

        //if variable ends with *, determine just their name
        string name2 = name;
        bool childsToo = false;
        if (name2.find(".*") != string::npos)
        {
            childsToo = true;
            name2 = name2.substr(0, name2.size()-2);
        }

        //load the valures
        auto values = readFromDb(name2, childsToo);
        

        //if nothing was found, add the default value to the result
        if (values.size() == 0)
            values.push_back(make_tuple(name2, defaultValue));

        //return the values

        return values;
    });
}

future<void> TheController::delVar(string varname)
{
    varname = "vars."+varname;

}

future<vector<string>> TheController::getChildsOfVar(string parentName)
{
    parentName = "vars."+parentName;

}


DynamicVar TheController::getVarInternalFlag(string vName, string flagName, DynamicVar defaultValue)
{ 
    if (flagName != "")
    {
        if (flagName[0] != '_');
            flagName = "_"+flagName;
        
        auto flagValue = this->getVar(vName + "."+flagName, defaultValue).get();
        return std::get<1>(flagValue[0]);
    }
}

void TheController::setVarInternalFlag(string vName, string flagName, DynamicVar value)
{
    if (flagName != "")
    {
        if (flagName[0] != '_');
            flagName = "_"+flagName;
        
        this->internalSetVar(vName + "."+flagName, value).get();
    }
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
    //For all variabels observed by this client, sent the current value
    atomic<bool> checkClientLiveTime = false;
    auto task = Utils::parallel_foreach<string>(controller_ClientHelper.getObservingVars(), [&](string currVarName)
    {
        auto currVarsAndValues = this->getVar(currVarName, "").get();

        //remove the 'vars.' from the begining of the varname

        if (checkClientLiveTime == false)
        {
            if (controller_ClientHelper.notify(currVarsAndValues) != API::ClientSendResult::LIVE)
                checkClientLiveTime = true;
        }
    }, this->tasker);

    task.wait();

    if (checkClientLiveTime)
        this->checkClientLiveTime(controller_ClientHelper);


}


void TheController::checkClientLiveTime(Controller_ClientHelper client)
{
    if (!client.isConnected())
    {
        if (!client.timeSinceLastLiveTime() >= maxTimeWaitingClient)
        {
            deleteClient(client);
        }
    }

}


void deleteClient(Controller_ClientHelper client)
{
    //remove from vars observations

    //delete the client
}