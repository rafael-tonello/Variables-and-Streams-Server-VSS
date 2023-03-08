#include  "Controller_VarHelper.h" 
 
Controller_VarHelper::Controller_VarHelper(ILogger *logger, StorageInterface* db, string varName)
{ 
    this->name = "vars."+varName;
    this->db = db;
    this->log = logger;
}
 
Controller_VarHelper::~Controller_VarHelper() 
{ 
     
} 

void Controller_VarHelper::setFlag(string flagName, DynamicVar value)
{
    
    if (flagName != "")
    {
        if (flagName[0] != '_');
            flagName = "_"+flagName;
        
        this->db->set(this->name + "." + flagName, value);
    }
}

DynamicVar Controller_VarHelper::getFlag(string flagName, DynamicVar defaultValue)
{
    if (flagName != "")
    {
        if (flagName[0] != '_');
            flagName = "_"+flagName;
        
        auto flagValue = this->db->get(this->name + "." + flagName, defaultValue);
        return flagValue;
    }
    
}

DynamicVar Controller_VarHelper::getValue(DynamicVar defaultValue)
{
    return db->get(name, defaultValue);
}

Errors::Error Controller_VarHelper::setValue(DynamicVar value)
{
    vector<future<void>> pendingTasks;

    if (name.find('*') != string::npos)
    {
        #ifdef __TESTING__
            Tester::global_test_result = Errors::Error_VariableWithWildCardCantBeSet;
        #endif
        log->warning("TheController", Errors::Error_VariableWithWildCardCantBeSet);
        //throw "Invalid setVar parameter. A variable with '*' can't be setted";

        return Errors::Error_VariableWithWildCardCantBeSet;
    }


    //set the variable
    db->set(name, value.getString());

    return Errors::NoError;
}

bool Controller_VarHelper::isLocked()
{
    auto lockValue = this->getFlag("lock", "0").getString();
    return lockValue != "0" && lockValue != "";
}

void Controller_VarHelper::lock()
{
        string finalVName = this->name + "._lock";
        //ir variable if currently locked, add an observer to it "._lock" property and wait the change of this to 0

        function<bool()> tryLockFunc = [&]()
        {
            //try lock var
            bool lockSucess = false;
            Utils::named_lock("varLockerSystem", [&]{
                if (this->getFlag("lock", "0").getString() == "0")
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
}

void Controller_VarHelper::unlock()
{
    Utils::named_lock("varLockerSystem", [&]{
        this->setFlag("lock", "0");
    });
}

vector<string> Controller_VarHelper::getChildsNames()
{
    vector<string> result;
    auto tmp =  db->getChilds(name);
    //filter flags
    for(auto &c: tmp)
        if (c != "" && c[0] != '_')
            result.push_back(c);

    return result;

}

bool Controller_VarHelper::isClientObserving(string clientId)
{
    auto result = db->hasValue(name + "._observers.byId."+clientId+".index");
    return result;
}

void Controller_VarHelper::addClientToObservers(string clientId, string customMetadata)
{
    
    runLocked([&](){

        int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();
        
        db->set(name + "._observers.list."+to_string(actualVar_observersCount), clientId);
        db->set(name + "._observers.byId."+clientId+".index", actualVar_observersCount);
        db->set(name + "._observers.byId."+clientId+".customMetadata", customMetadata);
        db->set(name + "._observers.list.count", actualVar_observersCount+1);
    });
}

void Controller_VarHelper::removeClientFromObservers(string clientId)
{
    runLocked([&](){

        int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();

        for (int c = actualVar_observersCount-1; c>=0; c--)
        {
            if (db->get(name + "._observers.list."+to_string(c), "").getString() == clientId)
            {
                for (int c2 = c; c2 < actualVar_observersCount-1; c2++)
                {
                    auto currId = db->get(name + "._observers.list."+to_string(c2+1), "").getString();
                    db->set(name+"._observers.list."+to_string(c2), currId);
                    db->set(name + "._observers.byId."+currId+".index", c2);
                }
                
                //remove the last item from the _observers.list
                db->deleteValue(name + "._observers.list."+to_string(actualVar_observersCount-1));

                actualVar_observersCount--;
                db->set(name + "._observers.list.count", actualVar_observersCount);

                //remove the item from _observers.byId
                db->deleteValue(name + "._observers.byId."+clientId+".index");
                db->deleteValue(name + "._observers.byId."+clientId+".customMetada");
            }
        }
    });
}

bool Controller_VarHelper::valueIsSetInTheDB()
{
    string invalidValue = "___NotFound___"; //"_n_o_t_NotFound_f_o_u_n_d_"
    return this->getValue(invalidValue).getString() != invalidValue;
}

void Controller_VarHelper::foreachObserversClients(FObserversForEachFunction f)
{
    auto clientIds = getObserversClientIds();
    for (auto &clientId: clientIds)
        f(clientId);
}

vector<string> Controller_VarHelper::getObserversClientIds()
{
    vector<string> result;
    int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();

    for (int c = 0; c < actualVar_observersCount; c++)
    {
        auto clientId = db->get(name + "._observers.list."+to_string(c), "").getString();
        if (clientId != "")
        {
            result.push_back(clientId);
        }
    }

    return result;
}

vector<tuple<string, string>> Controller_VarHelper::getObserversClientIdsAndMetadta()
{
    vector<string> clients = this->getObserversClientIds();
    vector<tuple<string, string>> result;

    for (auto &c : clients)
    {
        auto customMetadata = db->get(name + "._observers.byId."+c+".customMetada", "").getString();
        result.push_back(std::make_tuple(c, customMetadata));
    }

    return result;
}

void Controller_VarHelper::deleteValueFromDB()
{
    db->deleteValue(name, false);
}


void Controller_VarHelper::runLocked(function<void()>f)
{
    //Utils::named_lock("internal.observationLock", [&](){ //less memory, lock all variables at same time
    Utils::named_lock(name + "._observationLock", [&](){ //more memory, lock individualy each variable
        f();
    });
}

string Controller_VarHelper::getMetadataForClient(string clientId)
{
    auto customMetadata = db->get(name + "._observers.byId."+clientId+".customMetada", "").getString();

    return customMetadata;
}