#ifndef _InMemoryConfProvider_h_
#define _InMemoryConfProvider_h_
#include <vector>
#include <tuple>
#include <string>
#include <functional>
#include <map>
#include <IConfProvider.h>

using namespace std;

class InMemoryConfProvider: public IConfProvider
{
private:
    function<void(string, DynamicVar)> _onData = [](string a, DynamicVar b){};
    map<string, string> vars;

    void callOnData()
    {
        for (auto &c: vars)
            _onData(c.first, c.second);
    }
public:
    //using IConfProvider_onData = function<void (vector<tuple<string, string>> configurations)>;
    InMemoryConfProvider(vector<tuple<string, string>> initialConfiguration)
    {
        for(auto &c: initialConfiguration)
            vars[std::get<0>(c)] = vars[std::get<1>(c)];
    }

    InMemoryConfProvider(){}

    void setOrUpdateConfig(string name, string value)
    {
        vars[name] = value;
        callOnData();
    }


    void listen(function<void(string, DynamicVar)> f) override {
        _onData = f;
        callOnData();
    }

    bool contains(string name) override {
        return vars.count(name) > 0;
    };

    DynamicVar get(string name) override {
        if (vars.count(name))
            return vars[name];
        return DynamicVar("");
    }

    ~InMemoryConfProvider()
    {
        vars.clear();
    }



    string getTypeIdName()
    {
        return typeid(InMemoryConfProvider).name();
    }

    //Confs will use this method to inform variables for each alias created. It can be util for this like       
    //variable monitoring processes
    void informPotentialUsableVariable(string varName) override {};

};

#endif