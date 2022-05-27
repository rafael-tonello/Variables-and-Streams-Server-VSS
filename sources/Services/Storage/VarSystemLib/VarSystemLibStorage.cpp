#include  "VarSystemLibStorage.h" 
 
VarSystemLibStorage::VarSystemLibStorage(DependencyInjectionManager* dim) 
{ 
    this->confs = dim->get<Config>();

    confs->observate("varsDbDirectory", [&](DynamicVar value)
    {
        db = shared_ptr<FileVars>(new FileVars(value.getString(), true));
    }, "~/.local/VSS/varsDb");
} 
 
void VarSystemLibStorage::set(string name, DynamicVar v)
{
    db->set(name, v.getString());
}

DynamicVar VarSystemLibStorage::get(string name, DynamicVar defaultValue)
{
    return DynamicVar(db->get(name, defaultValue.getString()).AsString());
}

vector<string> VarSystemLibStorage::getChilds(string parentName)
{
    return db->getChilds(parentName);
}

bool VarSystemLibStorage::hasValue(string name)
{
    return db->get(name, "__iNval!d__").AsString() != "__iNval!d__";
}

void VarSystemLibStorage::deleteValue(string name, bool deleteChildsInACascade)
{

    if (deleteChildsInACascade)
    {
        auto childs = getChilds(name);
        for(auto &currChild: childs)
            deleteValue(name + "." + currChild, true);
    }
    
    db->del(name);

    return true;
}

void VarSystemLibStorage::forEachChilds(string parentName, function<void(string, DynamicVar)> f)
{
    auto names = this->getChilds(parentName);
    for (auto &c: names)
    {
        f(c, this->get(c, ""));
    }
}

future<void> VarSystemLibStorage::forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel) 
{
    auto names = this->getChilds(parentName);
    vector<future<void>> pendingTasks;
    for (auto &c: names)
    {
        pendingTasks.push_back(taskerForParallel->enqueue([&](string name){
            f(name, this->get(name, ""));
        }, c));
    }

    return taskerForParallel->enqueue([&](vector<future<void>> pendingTasks2){
        for (auto &c: pendingTasks2)
            c.wait();

        return;
    }, std::move(pendingTasks));
    
}