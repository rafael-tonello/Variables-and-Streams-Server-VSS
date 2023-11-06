#ifndef __SOENVIRONMENTCONFPROVIDER__H__ 
#define __SOENVIRONMENTCONFPROVIDER__H__ 

#include "IConfProvider.h"
#include <atomic>
#include <vector>
#include <mutex>
#include <unistd.h>
#include <thread>
 
class SoEnvironmentConfProvider: public IConfProvider { 
public: 
    SoEnvironmentConfProvider(); 
    ~SoEnvironmentConfProvider(); 

    function<void(string, DynamicVar)> listenF = [](string a, DynamicVar b){};
    vector<string> varsNames;
public:
    /* IConfProvider interface */
    bool contains(string name) override;
    DynamicVar get(string name) override;
    void listen(function<void(string, DynamicVar)> f) override;
    string getTypeIdName() override;

    //Confs will use this method to inform variables for each one alias was created. It can be util for this like       
    //variable monitoring processes
    void informPotentialUsableVariable(string varName) override;
}; 
 
#endif 
