#include "inmemorydb.h"

InMemoryDB::InMemoryDB(DependencyInjectionManager* dim)
{
    
}

InMemoryDB::~InMemoryDB()
{
    
}

void InMemoryDB::set(string name, DynamicVar v)
{
    db[name] = v;
    
}

DynamicVar InMemoryDB::get(string name, DynamicVar defaultValue)
{
    if (db.count(name))
        return db[name];

    return defaultValue;
}

vector<string> InMemoryDB::getChilds(string parentName)
{
    vector<string> ret;
    std::set<string> foundOnes;

    for (auto &c: db)
    {
        auto key = c.first;

        if (key.size() > parentName.size() && key.substr(0, parentName.size()) == parentName)
        {
            key = key.substr(parentName.size());
            if (key.size() > 0 && key[0] == '.')
                key = key.substr(1);
                
            if (auto pos = key.find('.'); pos != string::npos)
                key = key.substr(0, pos);
                
            if (foundOnes.count(key) == 0)
                foundOnes.insert(key);
        }

    }

    for (auto &c: foundOnes)
        ret.push_back(c);

    return ret;
}

bool InMemoryDB::hasValue(string name)
{
    return db.count(name) > 0;
}

void InMemoryDB::deleteValue(string name, bool deleteChildsInACascade)
{
    if (db.count(name))
        db.erase(name);

    if (deleteChildsInACascade)
    {
        auto childs = this->getChilds(name);
        for (auto &c: childs)
            deleteValue(c);
    }
}

