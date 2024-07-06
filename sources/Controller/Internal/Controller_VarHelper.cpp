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
    if (name.find('*') != string::npos)
    {
        #ifdef __TESTING__
            Tester::global_test_result = Errors::Error_VariableWithWildCardCantBeSet;
        #endif
        log->error("TheController", Errors::Error_VariableWithWildCardCantBeSet);
        //throw "Invalid setVar parameter. A variable with '*' can't be setted";

        return Errors::Error_VariableWithWildCardCantBeSet;
    }


    //set the variable
    db->set(name, value.getString());

    return Errors::NoError;
}

bool Controller_VarHelper::isLocked()
{
    auto  lockValue = this->getFlag("lock", "0").getString();

    return lockValue != "0" && lockValue != "";
}

Errors::Error Controller_VarHelper::lock(uint maxTimeOut_ms)
{
    log->debug("Locking var "+this->name);
    maxTimeOut_ms *= 1000;
        
    uint timeout_count = 0;
    string finalVName = this->name + "._lock";
    //ir variable if currently locked, add an observer to it "._lock" property and wait the change of this to 0

    function<bool()> tryLockFunc = [=]()
    {
        //try lock var
        bool lockSucess = false;
        Utils::named_lock("varLockerSystem", [&]{
            
            if (!this->isLocked())
            {
                //set the var property '._lock' to 1 (use setVar to change the ._lock)
                db->set(finalVName, "1");
                auto writeValue = db->get(finalVName, "0").getString();
                lockSucess = writeValue == "1";
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
        {
            if (timeout_count >= maxTimeOut_ms)
                return Errors::Error_TimeoutReached;

            auto sleepTime = 1000 + rand() % 9000;
            timeout_count += sleepTime;
            usleep(sleepTime);
        }
    }

    return Errors::NoError;
}

void Controller_VarHelper::unlock()
{
    log->debug("UnLocking var "+this->name);
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

bool Controller_VarHelper::isObserving(string clientId, string metadata)
{
    auto result = db->hasValue(name + "._observers.byId."+clientId+".byMetadata."+metadata);
    return result;
}

void Controller_VarHelper::addObserver(string clientId, string metadata)
{
    
    runLocked([&](){

        int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();
        
        db->set(R("?._observers.list.?.clientId", {name, to_string(actualVar_observersCount)}), clientId);
        db->set(R("?._observers.list.?.metadata", {name, to_string(actualVar_observersCount)}), metadata);

        db->set(R("?._observers.byId.?.byMetadata.?", {name, clientId, metadata}), actualVar_observersCount);
        db->set(R("?._observers.list.count", {name}), actualVar_observersCount+1);
    });
}

void Controller_VarHelper::removeCliObservings(string clientId)
{
    runLocked([&](){

        int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();

        for (int c = actualVar_observersCount-1; c>=0; c--)
        {
            auto currItemName = db->get(R("?._observers.list.?.clientId", {name, to_string(c)}), "").getString();
            auto currMetadata = db->get(R("?._observers.list.?.metadata", {name, to_string(c)}), "").getString();
            if ( currItemName == clientId)

            removeObserving(currItemName, currMetadata);
        }
    });
}

void Controller_VarHelper::removeObserving(string clientId, string metadata)
{
    runLocked([&](){

        int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();

        for (int c = actualVar_observersCount-1; c>=0; c--)
        {
            auto currItemName = db->get(R("NM._observers.list.IDX.clientId", {{"NM", name}, {"IDX", to_string(c)}}), "").getString();
            auto currMetadata = db->get(R("NM._observers.list.IDX.metadata", {{"NM", name}, {"IDX", to_string(c)}}), "").getString();
            if ( currItemName == clientId && currMetadata == metadata)
            {
                for (int c2 = c; c2 < actualVar_observersCount-1; c2++)
                {
                    auto currId = db->get(R("?._observers.list.?.clientId", {name, to_string(c2+1)}), "").getString();
                    auto currMetadata = db->get(R("?._observers.list.?.metadata", {name, to_string(c2+1)}), "").getString();

                    db->set(R("?._observers.list.?.clientId", {name, to_string(c2)}), currId);
                    db->set(R("?._observers.list.?.metadata", {name, to_string(c2)}), currMetadata);
                    db->set(R("?._observers.byId.?.byMetadata.?", {name, currId, currMetadata}), c2);
                }
                
                //remove the last item from the _observers.list
                db->deleteValue(R("?._observers.list.?", {name, to_string(actualVar_observersCount-1)}), true);

                actualVar_observersCount--;
                db->set(name + "._observers.list.count", actualVar_observersCount);

                //remove the item from _observers.byId
                db->deleteValue(R("?._observers.byId.?", {name, clientId}), true);
            }
        }
    });
}

bool Controller_VarHelper::valueIsSetInTheDB()
{
    string invalidValue = "___NotFound___"; //"_n_o_t_NotFound_f_o_u_n_d_"
    return this->getValue(invalidValue).getString() != invalidValue;
}

void Controller_VarHelper::foreachObservations(FObservationsForEachFunction f)
{
    auto observations = getObservations();
    for (auto &curr: observations)
        f(std::get<0>(curr), std::get<1>(curr));
}

vector<tuple<string, string>> Controller_VarHelper::getObservations()
{
    vector<tuple<string, string>> result;
    int actualVar_observersCount = db->get(name + "._observers.list.count", 0).getInt();

    for (int c = 0; c < actualVar_observersCount; c++)
    {
        auto clientId = db->get(R("?._observers.list.?.clientId", {name, to_string(c)}), "").getString();
        auto metadata = db->get(R("?._observers.list.?.metadata", {name, to_string(c)}), "").getString();
        if (clientId != "")
            result.push_back({clientId, metadata});
    }

    return result;
}

vector<tuple<string, string>> Controller_VarHelper::getObservationsOfAClient(string clientId)
{
    vector<tuple<string, string>> result;
    auto childs = db->getChilds(R("?._observers.byId.?.byMetadata", {name, clientId}));
    for (auto &curr: childs)
    {
        if (curr.find('.') != string::npos)
        {
            auto metadata = curr.substr(curr.find('.') + 1);
            result.push_back({clientId, metadata});
        }
    }

    return result;
}

vector<string> Controller_VarHelper::getMetadatasOfAClient(string clientId)
{
    vector<string> result;

    //auto childs = db->getChilds(R("{name}._observers.byId.{cliId}.byMetadata", {{"{name}", name}, {"{cliId}", clientId}}));
    auto childs = db->getChilds(R("?._observers.byId.?.byMetadata", {name, clientId}));
    for (auto &curr: childs)
    {
        if (curr.find('.') != string::npos)
        {
            auto metadata = curr.substr(curr.find('.') + 1);
            result.push_back(metadata);
        }
    }
    return result;
}

//return true ans was deleted and false otherwise
bool Controller_VarHelper::deleteValueFromDB()
{
    if (db->get(name, "___NotFound___") == "___NotFound___")
        return false;

    db->deleteValue(name, false);
    return true;
}


void Controller_VarHelper::runLocked(function<void()>f)
{
    //Utils::named_lock("internal.observationLock", [&](){ //less memory, lock all variables at same time
    Utils::named_lock(name + "._observationLock", [&](){ //more memory, lock individualy each variable
        f();
    }, 10000);
}
