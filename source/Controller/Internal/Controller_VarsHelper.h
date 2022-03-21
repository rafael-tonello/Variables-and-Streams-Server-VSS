#ifndef __CONTROLLER_VARSHELPER__H__ 
#define __CONTROLLER_VARSHELPER__H__ 

#include <iostream>
#include <string>
#include <DynamicVar.h>

using namespace std;
using namespace Shared;

class Controller_VarsHelper { 
private:
    string name;
public: 
    Controller_VarsHelper(string varName); 
    ~Controller_VarsHelper(); 

    bool isLocked();
    bool lock();
    bool unlock();

    void setFlag(DynamicVar flag);
    DynamicVar getFlag(DynamicVar defaultValue = "");
}; 
 
#endif 
