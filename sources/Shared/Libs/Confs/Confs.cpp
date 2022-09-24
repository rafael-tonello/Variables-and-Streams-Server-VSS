#include "Confs.h"

Shared::Config::Config(shared_ptr<IConfigurationProvider> provider)
{
    this->confProvider = provider;

    this->confProvider->readAndObservate([&](vector<tuple<string, string>> configurations){
        this->processConfigs(configurations);
    });
}

void Shared::Config::processConfigs(vector<tuple<string, string>> configurations)
{
    //read all configuration from the configuration provider
    string name, value;

    for (auto &c : configurations)
    {
        name = std::get<0>(c);
        value = applyPlaceHolders(std::get<1>(c));

        if (this->vars.count(name) > 0)
        {
            //checks if the value was changed
            if (this->vars[name].lastValue.getString() != value)
            {
                //update the value
                auto newValue = DynamicVar(value);
                this->vars[name].lastValue = newValue;
                //notify observers
                for (auto &currObserver: this->vars[name].observers)
                    currObserver(newValue);
            }
        }
        else
        {
            //create a new var in the memory
            this->vars[name] = {.lastValue = DynamicVar(value)};
        }

    }

}

DynamicVar Shared::Config::get(string varName, DynamicVar defaultValue)
{
    if (this->vars.count(varName) > 0)
        return this->vars[varName].lastValue;
    return DynamicVar(applyPlaceHolders(defaultValue.getString()));
}

void Shared::Config::observate(string varName, ObserveFunction onVarChanged, DynamicVar defaultValueIfVarNotExists, bool forceFirstCall)
{
    defaultValueIfVarNotExists =  DynamicVar(applyPlaceHolders(defaultValueIfVarNotExists.getString()));
    //check if variable already exists. If not, creates them
    if (this->vars.count(varName) == 0)
        this->vars[varName] = {.lastValue = defaultValueIfVarNotExists};

    //add the observer to the observer lists
    this->vars[varName].observers.push_back(onVarChanged);

    //cals by the first time
    if (forceFirstCall)
        onVarChanged(this->vars[varName].lastValue);
}

void Shared::Config::createPlaceHolder(string placeHolder, string value)
{
    placeHolders.push_back(std::make_tuple(placeHolder, value));
}

string Shared::Config::applyPlaceHolders(string value)
{
    for (auto &c: placeHolders)
    {
        string pName = std::get<0>(c);
        string pValue = std::get<1>(c);
        auto pos = value.find(pName);
        if (pos != string::npos)
        {
            value = value.substr(0, pos) + pValue + value.substr(pos + pName.size());
        }
    }

    return value;
}