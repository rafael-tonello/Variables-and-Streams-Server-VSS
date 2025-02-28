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

using FObservationsForEachFunction = function<void(string currentClientId, string metadata)>;
#define R Utils::sr
#define RM Utils::srm

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
    //return true ans was deleted and false otherwise
    bool deleteValueFromDB();
    vector<string> getChildsNames();
    bool isObserving(string clientId, string metadata);
    void addObserver(string clientId, string metadata);
    void removeCliObservings(string clientId);
    void removeObserving(string clientId, string metadata);
    void _internal_removeObserving(string clientId, string metadata);
    void foreachObservations(FObservationsForEachFunction f);

    /// @brief Return a vector with client ids and metadata
    /// @return a vector of tuples where each is like: tuple<clientId, metadata>
    vector<tuple<string, string>> getObservations();

    vector<tuple<string, string>> getObservationsOfAClient(string clientId);
    vector<string> getMetadatasOfAClient(string clientId);

}; 
 
#endif 
