#include "ramcachedb.h"

RamCacheDB::RamCacheDB(DependencyInjectionManager* dim)
{ 
    this->confs = dim->get<Confs>();
    this->log = dim->get<ILogger>()->getNamedLoggerP("RamCacheDBStorage");

    confs->listenA("DbDirectory", [&](DynamicVar value)
    {
        this->log->info("Database directory: " + value.getString());
        this->dataDir = value.getString();
        this->load();
    });

    confs->listenA("RamCacheDbDumpIntervalMs", [&](DynamicVar value)
    {
        this->log->info("Dump interval setted to: " + value.getString() + "ms");
        this->dumpIntervalMs = value.getInt();
    });


    dumpThread = new thread([=](){
        int waitingtotalTimeMs=0;
        while (continueRunning)
        {
            if (waitingtotalTimeMs > dumpIntervalMs) //5 minutes
            {
                dump();
                waitingtotalTimeMs=0;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
            waitingtotalTimeMs+=50;
        }
    });
}

RamCacheDB::~RamCacheDB()
{
    continueRunning = false;
    this->dumpThread->join();
    dump();
}

RamCacheDBItem* RamCacheDB::_scrollTree(string name, RamCacheDBItem &curr, bool readOnly)
{
    auto currName = name;
    string remainingName = "";
    if (name.find('.') != string::npos)
    {
        currName = name.substr(0, name.find('.'));
        remainingName = name.substr(name.find('.')+1);
    }

    if (curr.childs.count(currName) == 0)
    {
        if (readOnly)
            return nullptr;

        curr.childs[currName] = RamCacheDBItem();
        curr.childs[currName].imediateName = currName;
        curr.childs[currName].fullName = curr.fullName == "" ? currName : curr.fullName + "." + currName;
        curr.childs[currName].parent = &curr;

    }

    if (remainingName == "")
    {
        return &curr.childs[currName];
    }

    return _scrollTree(remainingName, curr.childs[currName], readOnly);
    
}

void RamCacheDB::set(string name, DynamicVar v)
{
    dblocker.lock();
    auto item = _scrollTree(name, root, false);
    if (item != nullptr)
    {
        item->value = v;
        pendingChanges = true;
    }
    else
    {
        dblocker.unlock();
        throw runtime_error("Error trying to set value on RamCacheDB, item not found: " + name);
    }

    dblocker.unlock();
}



DynamicVar RamCacheDB::get(string name, DynamicVar defaultValue)
{
    auto ret = defaultValue;
    dblocker.lock();
    auto item = _scrollTree(name, root, true);
    if (item != nullptr)
        ret = item->value;
        
    dblocker.unlock();
    return ret;
}


//return only imediate childs, do not return subschilds (childs of childs). Return only imediate key name (the full key name should not be returned)
vector<string> RamCacheDB::getChilds(string parentName)
{
    vector<string> ret;
    
    dblocker.lock();
    auto item = _scrollTree(parentName, root, true);
    if (item != nullptr)
    {
        for (auto &c: item->childs)
            ret.push_back(c.first);
    }
    dblocker.unlock();

    return ret;
}

bool RamCacheDB::hasValue(string name)
{
    dblocker.lock();
    auto item = _scrollTree(name, root, true);
    dblocker.unlock();
    return item != nullptr;
}

void RamCacheDB::deleteValue(string name, bool deleteChildsInACascade)
{
    dblocker.lock();
    auto item = _scrollTree(name, root, false);
    if (item != nullptr)
    {
        item->value = DynamicVar();

        if (deleteChildsInACascade)
            item->childs.clear();

        if (item->childs.size() == 0){
            item->parent->childs.erase(item->imediateName);
        }
        pendingChanges = true;
    }
    dblocker.unlock();
}

void RamCacheDB::forEachChilds(string parentName, function<void(string, DynamicVar)> f)
{
    dblocker.lock();
    auto item = _scrollTree(parentName, root, true);
    if (item != nullptr)
    {
        for (auto &c: item->childs)
            f(parentName + "." + c.first, c.second.value);
    }
    dblocker.unlock();

}

future<void> RamCacheDB::forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel)
{
    dblocker.lock();
    auto item = _scrollTree(parentName, root, true);
    if (item != nullptr)
    {
        for (auto &c: item->childs)
            taskerForParallel->enqueue([=](){f(parentName + "." + c.first, c.second.value); });
    }
    dblocker.unlock();

    std::promise<void> p;
    p.set_value();
    return p.get_future();
}

string RamCacheDB::dumpToString(RamCacheDBItem &current, string currentParentName)
{
    string ret = "";
    if (currentParentName != "")
        currentParentName = currentParentName + ".";
    for (auto &c: current.childs)
    {
        if (c.second.value.getString() !=  "")
            ret += currentParentName + c.first + "=" + c.second.value.getString() + "\n";

        ret += dumpToString(c.second, currentParentName +  c.first);
    }
    return ret;
}

void RamCacheDB::dump()
{
    if (!pendingChanges)
        return;

    pendingChanges = false;


    
    this->log->debug("Dumping database to disk");
    dblocker.lock();
    string fileText=dumpToString(root);
    dblocker.unlock();
    
    //create directory
    Utils::ssystem("mkdir -p \"" + dataDir+"\"");

    Utils::writeTextFileContent(dataDir + "/"+ DUMP_FILE_NAME, fileText);
    this->log->debug("Dumping database to disk finished");
}

string getDumpFileName();


void RamCacheDB::load()
{
    this->log->info("Loading database from disk");
    string fileText = Utils::readTextFileContent(getDumpFilePath());
    auto lines = Utils::splitString(fileText, "\n");
    dblocker.lock();
    for (auto &c: lines)
    {
        auto parts = Utils::splitString(c, "=");
        if (parts.size() == 2)
        {
            auto item = _scrollTree(parts[0], root, false);
            if (item != nullptr)
                item->value = parts[1];
            else
            {
                dblocker.unlock();
                throw runtime_error("RamCacheDB Error trying to loading a value from the disk, item not found: " + parts[0]);
            }
        }
    }
    dblocker.unlock();
    this->log->info("Loading database from disk finished");
}

string RamCacheDB::getDumpFilePath()
{
    return dataDir + "/"+ DUMP_FILE_NAME;
}
