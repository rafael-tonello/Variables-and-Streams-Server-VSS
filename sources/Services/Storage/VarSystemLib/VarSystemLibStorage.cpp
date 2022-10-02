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
    v = escape(v);
    db->set(name, v.getString());
}

DynamicVar VarSystemLibStorage::get(string name, DynamicVar defaultValue)
{

    return DynamicVar(unescape(db->get(name, defaultValue.getString()).AsString()));
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
    vector<future<void>> *pendingTasks = new vector<future<void>>;
    for (auto &c: names)
    {
        pendingTasks->push_back(taskerForParallel->enqueue([&](string name){
            f(name, this->get(name, ""));
        }, c));
    }

    auto ret = taskerForParallel->enqueue([&](){
        for (auto &c: *pendingTasks)
            c.wait();

        pendingTasks->clear();
        delete pendingTasks;
    });

    return ret;
    
}

string VarSystemLibStorage::escape(string text)
{
    text = Utils::stringReplace(text, {
        std::make_tuple<string, string>("*", "__wildcard__")
    });

    return text;

}

string VarSystemLibStorage::unescape(string text)
{
    text = Utils::stringReplace(text, {
        std::make_tuple<string, string>("__wildcard__", "*")
    });

    return text;
}
