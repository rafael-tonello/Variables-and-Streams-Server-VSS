#ifndef __IVAREXPORTER__H__ 
#define __IVAREXPORTER__H__ 

#include <string>
#include <vector>
#include <tuple>
#include <DynamicVar.h>

using namespace std;

 
class IVarsExporter { 
protected:
    map<string, DynamicVar> vars;
public: 
    virtual void add(string name, DynamicVar value)
    {
        this->vars[name] = value;
    };
    virtual void add(vector<tuple<string, DynamicVar>> varsToAdd)
    {
        for (auto &c: varsToAdd)
            this->add(std::get<0>(c), std::get<1>(c));
    }
    virtual void clear()
    {
        this->vars.clear();
    }

    virtual bool checkMymeType(string mimeType)
    {
        string tmp = this->getMimeType();
        std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), [](unsigned char c){ return std::tolower(c); });
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c){ return std::tolower(c); });

        return tmp == mimeType;
    }

    virtual operator string(){return toString(); }
    virtual operator const char*(){return toString().c_str(); }
public:
    virtual string toString() = 0;
    virtual string getMimeType() = 0;
}; 
 
#endif 
