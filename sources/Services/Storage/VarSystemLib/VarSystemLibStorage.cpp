#include  "VarSystemLibStorage.h" 
 
VarSystemLibStorage::VarSystemLibStorage(DependencyInjectionManager* dim) 
{ 
    this->confs = dim->get<Config>();
    this->log = dim->get<ILogger>()->getNamedLoggerP("VarSystemLibStorage");

    confs->observate("varsDbDirectory", [&](DynamicVar value)
    {
        this->log->info("Database directory: " + value.getString());
        this->databaseLocation = value.getString();
        db = shared_ptr<FileVars>(new FileVars(value.getString(), true));
    }, "~/.local/VSS/varsDb");
} 
 
void VarSystemLibStorage::set(string name, DynamicVar v)
{
    v = escape(v);
    db->set(name, v.getString());
}

DynamicVar VarSystemLibStorage::get(string name, DynamicVar defaultValue)
{
    name = escape(name);
    auto tmp = db->get(name, defaultValue.getString()).AsString();
    tmp = unescape(tmp);
    DynamicVar ret = DynamicVar(tmp);
    return ret;
}

vector<string> VarSystemLibStorage::getChilds(string parentName)
{
    parentName = escape(parentName);
    vector<string> result;
    auto tmp = db->getChilds(parentName);
    for (auto &c: tmp)
        result.push_back(unescape(c));

    return result;
}

bool VarSystemLibStorage::hasValue(string name)
{
    name = escape(name);
    return db->get(name, "__iNval!d__").AsString() != "__iNval!d__";
}

void VarSystemLibStorage::deleteValue(string name, bool deleteChildsInACascade)
{
    name = escape(name);

    if (deleteChildsInACascade)
    {
        auto childs = getChilds(name);
        for(auto &currChild: childs)
            deleteValue(name + "." + currChild, true);
    }
    
    db->del(name);
}

void VarSystemLibStorage::forEachChilds(string parentName, function<void(string, DynamicVar)> f)
{
    parentName = escape(parentName);
    
    auto names = this->getChilds(parentName);
    for (auto &c: names)
    {
        f(c, this->get(c, ""));
    }
}

future<void> VarSystemLibStorage::forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel) 
{
    parentName = escape(parentName);

    auto names = this->getChilds(parentName);
    vector<future<void>> *pendingTasks = new vector<future<void>>;
    for (auto &c: names)
    {
        pendingTasks->push_back(taskerForParallel->enqueue([&](string name){
            f(name, this->get(name, ""));
        }, c));
    }

    auto ret = taskerForParallel->enqueue([&, pendingTasks](){
        for (auto &c: *pendingTasks)
            c.wait();

        pendingTasks->clear();
        delete pendingTasks;
    });

    return ret;
    
}

string VarSystemLibStorage::escape(string text)
{
    text = Utils::stringReplace(text, (vector<tuple<string, string>>){
        {"*", "__wildcard__"},
        {"\r", "__cr__"},
        {"\n", "__nl__"}
    });

    return text;

}

string VarSystemLibStorage::unescape(string text)
{
    text = Utils::stringReplace(text, (vector<tuple<string, string>>){
        {"__wildcard__", "*"},
        {"__cr__", "\r"},
        {"__nl__", "\n"}
    });

    return text;
}

string VarSystemLibStorage::getDatabseFolder()
{
    return databaseLocation;
}
