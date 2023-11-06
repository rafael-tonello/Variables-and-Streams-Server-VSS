#include "Confs.h"

Confs::Confs(vector<IConfProvider*> initialProviders)
{
    for (auto &c: initialProviders)
        this->addProvider(c);
}

Confs::~Confs()
{
    for (auto &c: providers)
        delete c;

    providers.clear();
}

Confs::ConfPlaceHolder::ConfPlaceHolder(Confs *ctrl):ctrl(ctrl){}

Confs::ConfPlaceHolder& Confs::ConfPlaceHolder::add(string replace, string by){
    ctrl->placeHolders.push_back({replace, by});
    return *this;
}

Confs::ConfPlaceHolder& Confs::ConfPlaceHolder::add(vector<tuple<string, string>> replacesAndBys){
    for (auto &c: replacesAndBys)
        this->add(std::get<0>(c), std::get<1>(c));
    
    return *this;
}

//Warning! Providers will be automatically deleted whend the Confs isntance be destroyed
void Confs::addProvider(IConfProvider *provider)
{
    string typeIdName = provider->getTypeIdName();
    this->providers.push_back(provider);
    provider->listen([&, typeIdName](string confName, DynamicVar newValue){
        this->confChangedInAProvider(typeIdName, confName, newValue);
    });
}

void Confs::confChangedInAProvider(string providerName, string name, DynamicVar value)
{
    for (auto &alias: aliases)
    {
        for (auto &provider: alias.second.optionalNamesInEachProviders)
        {
            if (provider.first == providerName)
            {
                for (auto &possibleName: provider.second)
                {
                    if (possibleName == name)
                    {
                        //request value using getA function. This function scrolls the providers in importance order. If use 'value' of 'confChangedInAProvier, the importance will be ignored
                        auto lastValue = alias.second.lastValidValue;
                        auto newValue = getA(alias.first);
                        if (lastValue.getString() != newValue.getString())
                        {
                            for(auto &observer: alias.second.listeners)
                                observer(applyPlaceHolders(newValue));
                        }

                    }
                }
            }
        }
    }
}

//return a ConfAliaser object, that can be used to construct an alias. The alias allow you to identify diferent names for same configuration.
Confs::ConfAliaser Confs::createAlias(string aliasName)
{
    auto ret = ConfAliaser(this, aliasName);
    return ret;
}

Confs::ConfPlaceHolder Confs::createPlaceHolders()
{
    auto ret = ConfPlaceHolder(this);
    return ret;
}

//identify the alias and scrolls over all providers. The first one with the configuration will be used to return the value.
DynamicVar Confs::getA(string alias, DynamicVar defaultValue)
{
    if (aliases.count(alias) > 0)
    {
        auto &aliasInfo = aliases[alias];
        for (auto &provider: providers)
        {
            if (aliasInfo.optionalNamesInEachProviders.count(provider->getTypeIdName()))
            {
                auto namesInThisProvider = aliasInfo.optionalNamesInEachProviders[provider->getTypeIdName()];
                for (auto &possibleName : namesInThisProvider)
                {
                    if (provider->contains(possibleName))
                    {
                        aliasInfo.lastValidValue = provider->get(possibleName);
                        return applyPlaceHolders(aliasInfo.lastValidValue);
                    }
                }
            }
        }
    }
    return defaultValue;
}

void Confs::listenA(string alias, function<void(DynamicVar)> f, bool callFImedially, DynamicVar defaultValueForImediateFCall)
{
    if (!aliases.count(alias))
        aliases[alias] = ConfAliasInfo();

    aliases[alias].listeners.push_back(f);
    if (callFImedially)
        f(getA(alias, defaultValueForImediateFCall));
}

string Confs::applyPlaceHolders(string source)
{
    return Utils::stringReplace(source, this->placeHolders);
}

void Confs::informPotentialVariables(string providerTName, vector<string>possibleNames)
{
    for (auto &c: providers)
    {
        if (c->getTypeIdName() == providerTName)
        {
            for (auto potetialVar: possibleNames)
            {
                c->informPotentialUsableVariable(potetialVar);
            }
        }
    }
}