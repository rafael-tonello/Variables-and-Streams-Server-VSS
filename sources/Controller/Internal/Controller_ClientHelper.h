#ifndef __CLIENTINFO__H__ 
#define __CLIENTINFO__H__

#include <StorageInterface.h>
#include <ApiInterface.h>
#include <string>
#include <DynamicVar.h>
#include <utils.h>
#include <logger.h>
#include "Controller_VarHelper.h"

using namespace std;
using namespace API;

 

enum Controller_ClientHelperError {NO_ERROR, API_NOT_FOUND};

class Controller_ClientHelper{
private:
    StorageInterface *db = NULL;
    ApiInterface* api = NULL;
    string clientId;
    void initialize();
    int findVarIndexOnObservingVars(string varName);
public:
    Controller_ClientHelper(StorageInterface *db, string clientId, ApiInterface* api);
    Controller_ClientHelper(StorageInterface *db, string clientId, map<string, ApiInterface*> apis, Controller_ClientHelperError &error);
    int64_t getLastLiveTime();
    int64_t timeSinceLastLiveTime();
    void updateLiveTime();
    bool isConnected();
    vector<string> getObservingVars();
    int getObservingVarsCount();
    API::ClientSendResult notify(vector<tuple<string, DynamicVar>> varsAndValues);
    void registerNewObservation(string varName);
    void unregisterObservation(string varName);
    string getClientId();
    void removeClientFromObservationSystem();
};
 
#endif 
