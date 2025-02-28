#include "ramcachedb.h"

RamCacheDB::RamCacheDB()
{
    
}

RamCacheDB::~RamCacheDB()
{
    
}

void RamCacheDB::set(string name, DynamicVar v)
{
    this->db[name] = v;
}

DynamicVar RamCacheDB::get(string name, DynamicVar defaultValue)
{
    if (db.count(name))
        return db[name];
    return defaultValue;
}

//return only imediate childs, do not return subschilds (childs of childs). Return only imediate key name (the full key name should be returned)
vector<string> RamCacheDB::getChilds(string parentName)
{
    vector<string> ret;
    std::set<string> foundOnes;
    if (parentName.size() > 0 && parentName[parentName.size()-1] != '.')
        parentName = parentName + '.';
    for (auto &c: db)
    {
        auto key = c.first;
        if (key.size() > parentName.size() && key.find(parentName) == 0)
        {
            key=key.substr(parentName.size());

            auto pos = key.find('.');
            if (pos != string::npos)
                key = key.substr(0, pos);
                
            //key = parentName + key;
            if (foundOnes.count(key) == 0)
                foundOnes.insert(key);
        }
    }

    for (auto &c: foundOnes)
        ret.push_back(c);

    return ret;
}

bool RamCacheDB::hasValue(string name)
{
    return db.count(name) > 0;
}

void RamCacheDB::deleteValue(string name, bool deleteChildsInACascade)
{
    if (db.count(name))
    {
        db.erase(name);
        if (deleteChildsInACascade)
        {
            auto childs = getChilds(name);
            for (auto &c: childs)
                db.erase(c);
        }
    }
}

void RamCacheDB::forEachChilds(string parentName, function<void(string, DynamicVar)> f)
{
    auto childs = getChilds(parentName);
    for (auto &c: childs)
        f(c, db[c]);

}

future<void> RamCacheDB::forEachChilds_parallel(string parentName, function<void(string, DynamicVar)> f, ThreadPool *taskerForParallel)
{
    auto childs = getChilds(parentName);
    for (auto &c: childs)
        taskerForParallel->enqueue([=](){ f(c, db[c]); });
}
