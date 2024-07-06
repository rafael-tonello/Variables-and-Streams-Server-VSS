#ifndef __PREFIXTREEDB__H__
#define __PREFIXTREEDB__H__

#include <StorageInterface.h>
#include <prefixtreestorage/prefixtree.h>

class PrefixTreeDB: public StorageInterface{
public:
    PrefixTreeDB();
    ~PrefixTreeDB();

/* StorageInterface interface */
public:
    void set(string name, DynamicVar v) override;
    DynamicVar get(string name, DynamicVar defaultValue) override;
    vector<string> getChilds(string parentName) override;
    bool hasValue(string name) override;
    void deleteValue(string name, bool deleteChildsInACascade = false) override;
};

#endif
