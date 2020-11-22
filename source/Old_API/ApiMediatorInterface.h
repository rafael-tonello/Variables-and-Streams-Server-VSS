#ifndef _APIMEDIATORINTERFACE_H_
#define _APIMEDIATORINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
using namespace std;

namespace API
{
    class ApiMediatorInterface
    {
    public:
            virtual string GetVar(string name, string defaultValue) = 0;
            virtual void SetVar(string name, string value) = 0;
            virtual long GetVar_Int(string varname, long defaultValue) = 0;
            virtual void SetVar_Int(string varname, long value) = 0;
            virtual void DelVar(string varname) = 0;
            virtual vector<string> getChildsOfVar(string parentName) = 0;
            
    };
};
#endif
