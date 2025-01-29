#include "prefixtreedb.h"

PrefixThreeStorage::PrefixThreeStorage(DependencyInjectionManager *dep)
{
    init(dep);
};

PrefixThreeStorage::PrefixThreeStorage(DependencyInjectionManager *dep, bool useMemoryCache, size_t maxCacheSize = 0): useMemoryCache(useMemoryCache), maxCacheSize(maxCacheSize)
{
    init(dep);
}

PrefixThreeStorage::~PrefixThreeStorage()
{
    
}

void PrefixThreeStorage::init(DependencyInjectionManager *dep)
{
    this->tasks = dep->get<ThreadPool>();

    dep->get<Confs>()->listenA("DbDirectory", [=](DynamicVar dbDirectory){
        m.lock();
        if (tree != nullptr)
        {
            delete tree;
            delete fileStroage;
            tree = nullptr;
            fileStroage = nullptr;
        }

        fileStroage=new FileStorage(64, dbDirectory.getString()+"/database.db");
        tree =  new PrefixTree<DynamicVar>(fileStroage, [](string v){ return DynamicVar(v);}, [](DynamicVar v){ return v.getString();});

    });



    PrefixTree<DynamicVar> *tree = nullptr;
}

void PrefixThreeStorage::set(string name, DynamicVar v)
{
    m.lock();
    if (useMemoryCache)
    {
        if (maxCacheSize > 0 && currentCacheSize >= maxCacheSize)
            releaseMemory();

        cache[name] = v;

        this->tasks->enqueue([=](){
            tree->set(name, v);
        });
    }
    else
        tree->set(name, v);

    m.unlock();
}

DynamicVar PrefixThreeStorage::get(string name, DynamicVar defaultValue)
{
    m.lock();
    DynamicVar v = defaultValue;
    if (useMemoryCache)
    {
        if (cache.find(name) != cache.end())
            v = cache[name];
        else
        {
            v = tree->get(name, defaultValue);
            cache[name] = v;
        }
    }
    else
        v = tree->get(name, defaultValue);

    m.unlock();
    return v;

}


//return only imediate childs, do not return subschilds (childs of childs). Return only imediate key name (the full key name should be returned)
vector<string> PrefixThreeStorage::getChilds(string parentName)
{
    m.lock();
    vector<string> childs;
    auto allChilds = tree->searchChilds(parentName);
    for (auto &child: allChilds)
    {
        size_t index = parentName.size();
        size_t nextIndex = child.find(".", index);

        if (nextIndex != string::npos)
        {
            auto childName = child.substr(0, nextIndex);
            //check if childs alread contains the child
            if (find(childs.begin(), childs.end(), childName) == childs.end())
                childs.push_back(childName);
        }
    }
    m.unlock();
    return childs;
}

bool PrefixThreeStorage::hasValue(string name)
{
    if (useMemoryCache)
    {
        if (cache.find(name) != cache.end())
            return true;
    }

    return tree->contains(name);
}

void PrefixThreeStorage::deleteValue(string name, bool deleteChildsInACascade)
{
    m.lock();
    if (useMemoryCache)
    {
        if (cache.find(name) != cache.end())
            cache.erase(name);

        if (deleteChildsInACascade)
        {
            for (auto it = cache.begin(); it != cache.end();)
            {
                if (it->first.find(name) == 0)
                    cache.erase(it++);
                else
                    ++it;
            }
        }
    }

    tree->del(name);

    if (deleteChildsInACascade){
        auto childs = tree->searchChilds(name);
        for (auto &child: childs)
            tree->del(child);
    }

    m.unlock();
}
