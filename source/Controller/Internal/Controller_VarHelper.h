#ifndef __Controller_VarHelper__H__ 
#define __Controller_VarHelper__H__ 

#include <iostream>
#include <string>
#include <DynamicVar.h>
#include <StorageInterface.h>
#include <utils.h>
#include <logger.h>
#include <functional>

using namespace std;
using namespace Shared;

using FObserversForEachFunction = function<void(string currentClientId)>;

class Controller_VarHelper { 
private:
    string name;
    StorageInterface* db;
    ILogger *log;
public: 
    Controller_VarHelper(ILogger *logger, StorageInterface* db, string varName); 
    ~Controller_VarHelper(); 

    void setFlag(string flagName, DynamicVar value);
    DynamicVar getFlag(string flagName, DynamicVar defaultValue = "");
    DynamicVar getValue(DynamicVar defaultValue = "");
    void setValue(DynamicVar value);
    bool isLocked();
    bool valueIsSetInTheDB();
    void lock();
    void unlock();
    void deleteFromDB();
    vector<string> getChildsNames();
    bool isClientObserving(string clientId);
    void addClientToObservers(string clientId);
    void removeClientFromObservers(string clientId);
    void foreachObserversClients(FObserversForEachFunction f);
}; 
 
#endif 
