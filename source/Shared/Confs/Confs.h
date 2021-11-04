#include <string>
#include <DynamicVar.h>
#include <map>
#include <vector>
#include "./internal/IConfigurationProvider.h"
#include "./internal/SimpleConfFileProvider.h"
#include <memory>

using namespace std;
namespace Shared{
    using ObserveFunction = function<void(DynamicVar newValue)>;
    class Config_Variable_Info
    {
    public:
        DynamicVar lastValue;
        vector<ObserveFunction> observers;
    };
    class Config{
    private:
        shared_ptr<IConfigurationProvider> confProvider;
        map<string, Config_Variable_Info> vars;



        //this function read the entiry file and notify the variable observers
        void processConfigs(vector<tuple<string, string>> configurations);
    public:
        Config(shared_ptr<IConfigurationProvider> provider);
        Config(string fileName):Config(shared_ptr<IConfigurationProvider>(new SimpleConfFileProvider(fileName))){}

        DynamicVar get(string varName, DynamicVar defaultValue = "");
        void observate(string varName, ObserveFunction onVarChanged, DynamicVar defaultValueIfVarNotExists = "", bool forceFirstCall = true);
    };
}