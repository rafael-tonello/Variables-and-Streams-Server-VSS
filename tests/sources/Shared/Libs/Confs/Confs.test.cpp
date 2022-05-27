#include "Confs.test.h"
/*
this->test("'getting configuration' should return the correct value", [](){return false;});
this->test("'getting invalid configuration' should return a default value", [](){return false;});
this->test("observating a existing configuration", [](){return false;});
this->test("observating a inexistent configuration", [](){return false;});
this->test("observating a inexistent configuration and create it", [](){return false;});
*/


vector<string> ConfsTester::getContexts()
{
    return {"Confs"};
}

void ConfsTester::run(string context)
{
    if (context != "Confs")
        return;

    
    


}