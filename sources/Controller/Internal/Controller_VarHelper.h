#ifndef __Controller_VarHelper__H__ 
#define __Controller_VarHelper__H__ 

#include <iostream>
#include <string>
#include <DynamicVar.h>
#include <StorageInterface.h>
#include <utils.h>
#include <logger.h>
#include <functional>
#include <errors.h>
#include <limits.h>
#ifdef __TESTING__
    #include <tester.h>
#endif

using namespace std;

using FObserversForEachFunction = function<void(string currentClientId)>;

class Controller_VarHelper { 
private:
    #ifdef __TESTING__
        public:
    #endif
    string name;
    StorageInterface* db;
    ILogger *log;

    void runLocked(function<void()>f);
public: 
    Controller_VarHelper(ILogger *logger, StorageInterface* db, string varName); 
    ~Controller_VarHelper(); 

    void setFlag(string flagName, DynamicVar value);
    DynamicVar getFlag(string flagName, DynamicVar defaultValue = "");
    DynamicVar getValue(DynamicVar defaultValue = "");
    Errors::Error setValue(DynamicVar value);
    bool isLocked();
    bool valueIsSetInTheDB();
    Errors::Error lock(uint maxTimeOut_ms = UINT_MAX);
    void unlock();
    void deleteValueFromDB();
    vector<string> getChildsNames();
    bool isClientObserving(string clientId);
    void addClientToObservers(string clientId, string customMetadata);
    void removeClientFromObservers(string clientId);
    void foreachObserversClients(FObserversForEachFunction f);
    vector<string> getObserversClientIds();
    vector<tuple<string, string>> getObserversClientIdsAndMetadta();

    string getMetadataForClient(string clientId);
}; 
 
#endif 
