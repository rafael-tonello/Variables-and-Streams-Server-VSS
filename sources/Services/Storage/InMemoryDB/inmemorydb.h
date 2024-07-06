#ifndef __INMEMORYDB__H__
#define __INMEMORYDB__H__

#include <StorageInterface.h>
#include <dependencyInjectionManager.h>
#include <map>
#include <set>

using namespace std;

class InMemoryDB: public StorageInterface{
private:
    map<string, DynamicVar> db;
public:
    InMemoryDB(DependencyInjectionManager* dim);
    ~InMemoryDB();

    void set(string name, DynamicVar v) override;
    DynamicVar get(string name, DynamicVar defaultValue) override;
    vector<string> getChilds(string parentName) override;
    bool hasValue(string name) override;
    void deleteValue(string name, bool deleteChildsInACascade = false) override;
};

#endif
