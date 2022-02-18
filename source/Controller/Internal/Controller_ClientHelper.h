#ifndef __CLIENTINFO__H__ 
#define __CLIENTINFO__H__ ]

#include <StorageInterface.h>
#include <ApiInterface.h>
#include <string>
#include <DynamicVar.h>
#include <Utils.h>

using namespace Shared;
using namespace std;
using namespace API;

 

enum Controller_ClientHelperError {API_NOT_FOUND};
using Controller_ClientHelperOnError = function<void(Controller_ClientHelperError error)>;

class Controller_ClientHelper{
private:
    StorageInterface *db = NULL;
    ApiInterface* api = NULL;
    string clientId;
    void initialize();
public:
    Controller_ClientHelper(StorageInterface *db, string clientId, ApiInterface* api);
    Controller_ClientHelper(StorageInterface *db, string clientId, map<string, ApiInterface*> apis, Controller_ClientHelperOnError onError);
    int64_t getLastLiveTime();
    int64_t timeSinceLastLiveTime();
    void updateLiveTime();
    bool isConnected();
    vector<string> getObservingVars();
    API::ClientSendResult notify(vector<tuple<string, DynamicVar>> varsAndValues);
    void registerNewObservation(string varName);
    string getClientId();
};
 
#endif 
