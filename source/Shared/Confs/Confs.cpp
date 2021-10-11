#include "Confs.h"

Shared::Config::Config(shared_ptr<IConfigurationProvider> provider)
{
    this->confProvider = provider;
    this->reloadFile();
}

void Shared::Config::reloadFile()
{
    //read all configuration from the configuration provider
    auto currentConfigs = this->confProvider->readAllConfigurations();
    string name, value;

    for (auto &c : currentConfigs)
    {
        name = this->vars.count(std::get<0>(c));
        value = this->vars.count(std::get<1>(c));

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

Shared::DynamicVar Shared::Config::get(string varName, DynamicVar defaultValue)
{
    if (this->vars.count(varName) > 0)
        return this->vars[varName].lastValue;
    return defaultValue;
}

void Shared::Config::observate(string varName, ObserveFunction onVarChanged, DynamicVar defaultValueIfVarNotExists, bool forceFirstCall)
{
    //check if variable already exists. If not, creates them
    if (this->vars.count(varName) == 0)
        this->vars[varName] = {.lastValue = defaultValueIfVarNotExists};

    //add the observer to the observer lists
    this->vars[varName].observers.push_back(onVarChanged);

    //cals by the first time
    if (forceFirstCall)
        onVarChanged(this->vars[varName].lastValue);
}