#include  "prefixtreestorage.h" 


 
PrefixTreeStorage::PrefixTreeStorage(DependencyInjectionManager* dim) 
{
    this->confs = dim->get<Confs>();
    this->log = dim->get<ILogger>()->getNamedLoggerP("VarSystemLibStorage");

    confs->listenA("DbDirectory", [&](DynamicVar value)
    {
        value = value.getString() + "/vars.db.prefixtree";
        this->log->info("Database file: " + value.getString());
        this->dbFile = value.getString();
        
        dbStorage = new FileStorage(64, this->dbFile);
        ((FileStorage*)dbStorage)->init();
        db = new PrefixTree<DynamicVar>(
            dbStorage,
            [](string dt){ return DynamicVar(dt); },
            [](DynamicVar dt){ return dt.getString(); }
        );
        this->log->info2("Prefix tree database started!");
    }, "~/.local/VSS");

} 
 
PrefixTreeStorage::~PrefixTreeStorage() 
{ 
    delete db;    
    delete dbStorage;
} 
 
void PrefixTreeStorage::set(string name, DynamicVar v)
{
    if (!db->exists("index.keys." + name))
    {
        int count = db->get("index.indexes.count", 0);
        db->set("index.keys."+name, count);
        db->set("index.indexes."+std::to_string(count), name);
        db->set("index.indexes.count", count+1);
    }

    db->set("values."+name, v);
}

DynamicVar PrefixTreeStorage::get(string name, DynamicVar defaultValue)
{
    return db->get("values."+name, defaultValue);
}

vector<string> PrefixTreeStorage::getChilds(string parentName)
{
    log->warning("VarSystemLibStorage::getChilds, includesubchilds is not checked yet");
    vector<string> ret;
    std::set<string> foundOnes;
    auto count = db->get("index.indexes.count", 0).getInt();
    for (int c = 0; c < count ; c ++){
        auto key = db->get("index.index"+to_string(c), "").getString();
        if (key.size() > parentName.size() && key.find(parentName) == 0)
        {
            key=key.substr(parentName.size());

            auto pos = key.find('.');
            if (pos != string::npos)
                key = key.substr(0, pos);
                
            key = parentName + "." + key;
            if (foundOnes.count(key) == 0)
                foundOnes.insert(key);
        }
    }

    for (auto &c: foundOnes)
        ret.push_back(c);

    return ret;
}

bool PrefixTreeStorage::hasValue(string name)
{
    //return db->exists(name);
    return this->get(name, "").getString() != "";
}

void PrefixTreeStorage::deleteValue(string name, bool deleteChildsInACascade)
{
    //db->delete(name);
    this->set(name, "");

    //TODO: Delete childs here

}

