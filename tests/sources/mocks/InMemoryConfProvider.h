#ifndef _InMemoryConfProvider_h_
#define _InMemoryConfProvider_h_
#include <vector>
#include <tuple>
#include <string>
#include <functional>
#include <map>
#include <IConfigurationProvider.h>

using namespace std;
using namespace Shared;

class InMemoryConfProvider: public IConfigurationProvider
{
private:
    IConfigurationProvider_onData _onData = [](vector<tuple<string, string>> configurations){};
    map<string, string> vars;

    void callOnData()
    {
        vector<tuple<string, string>> varsAsTuple;
        for (auto &c: vars)
            varsAsTuple.push_back(std::make_tuple(c.first, c.second));

        _onData(varsAsTuple);
    }
public:
    //using IConfigurationProvider_onData = function<void (vector<tuple<string, string>> configurations)>;
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


    void readAndObservate(IConfigurationProvider_onData onData)
    {
        _onData = onData;
        callOnData();
    }

    ~InMemoryConfProvider()
    {
        vars.clear();
    }
};

#endif