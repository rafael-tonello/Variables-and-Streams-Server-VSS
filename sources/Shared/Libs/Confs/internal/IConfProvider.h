#ifndef _CONF_ICONFIGURATION_PROVIDER_H_
#define _CONF_ICONFIGURATION_PROVIDER_H_
#include <vector>
#include <tuple>
#include <string>
#include <functional>
#include <DynamicVar.h>

using namespace std;
class IConfProvider{
public:
    virtual bool contains(string name) = 0;
    virtual DynamicVar get(string name) = 0;
    virtual void listen(function<void(string, DynamicVar)> f) = 0;
    virtual string getTypeIdName() = 0;

    //Confs will use this method to inform variables for each one alias was created. It can be util for this like       
    //variable monitoring processes
    virtual void informPotentialUsableVariable(string varName) = 0;

};

#endif