#include  "soenvironmentconfprovider.h" 
 
SoEnvironmentConfProvider::SoEnvironmentConfProvider() 
{    
    
} 
 
SoEnvironmentConfProvider::~SoEnvironmentConfProvider() 
{ 
  
}
 
bool SoEnvironmentConfProvider::contains(string name)
{
    const char* envVar = getenv(name.c_str());

    return envVar != NULL;
}

DynamicVar SoEnvironmentConfProvider::get(string name)
{
    const char* envVar = getenv(name.c_str());
    string valueStr = "";
    if (envVar != NULL)
        valueStr = string(envVar);
    return DynamicVar(valueStr);
}

void SoEnvironmentConfProvider::listen(function<void(string, DynamicVar)> f)
{
    this->listenF = f;
    for (auto &c: varsNames)
        f(c, this->get(c));
}

string SoEnvironmentConfProvider::getTypeIdName()
{
    return typeid(SoEnvironmentConfProvider).name();
}

void SoEnvironmentConfProvider::informPotentialUsableVariable(string varName)
{
    this->varsNames.push_back(varName);
    listenF(varName, this->get(varName));
}
