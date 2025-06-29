#ifndef __CONFS_H__
#define __CONFS_H__

#include <string>
#include <DynamicVar.h>
#include <map>
#include <vector>
#include "./internal/IConfProvider.h"
#include <memory>
#include <tuple>
#include <functional>
#include <utils.h>

/**
 * @brief A observable cofiguration that can use many configuration sources.
 * 
 * To use this configuration system, follow this steps:
 *  1) instantiate the Configuration ans add providers of your choice. Note that providers should be added in importance order (most importante/relevant first)
 *  2) Configure the aliases for configurations you will use. Aliases is used to inform how each configuration can be named in each diferent source
 *  3) Request or observe the configuration using the configured 'alias'. If no alias was created for configuration, the Configuration will not be abble to find it.
 * 
 * 
 * 
 *  Example of context and usage:
 *      Lets supose your system needs a port for a socket conneciton. Ans also lets supose this port can be configured in a configuration file, in a environmnet variable and even can be informed in the command line of you program. 
 * 
 *      For this scenario, a possible use of Confs system can be:
 * 
 *      Confs confs({
 *          new CommandLineConfsProvider(argc, argv),
 *          new EnvoronmentVariablesConfProvider(),
 *          new SimpleConfFileProvider("/etc/myapp/myapp.conf")
 *      });
 * 
 *      confs.createAlias("port")
*           .add<CommandLineConfsProvider>({"--port", "-p"})
 *          .add<EnvoronmentVariablesConfProvider>({"MYAPP_PORT"})
 *          .add<SimpleConfFileProvider>({"port"})
 *      ;
 *      //or confs.createAlias("port").addForAnyProvider({"--port", "-p", "MYAPP_PORT", "port"});
 * 
 *      int port = confs.getA(port, 0).getInt();
 * 
 *  The Conf system will look for configurations in the order that the providers were added. The first provider that has the configuration will be used to return the value.
 * 
 */


using namespace std;
struct ConfAliasInfo{
    //map<providername, possible_names_of_the_conf_in_this_provider>
    map<string, vector<string>> optionalNamesInEachProviders;
    vector<function<void(DynamicVar)>> listeners = {};
    DynamicVar lastValidValue;
    DynamicVar defaultValue;
};

class Confs{
private:
    map<string, ConfAliasInfo> aliases;
    vector<IConfProvider*> providers;
    vector<tuple<string, string>> placeHolders;

    void confChangedInAProvider(string providerName, string name, DynamicVar value);
    void informPotentialVariables(string providerTName, vector<string>possibleNames);

    class ConfAliaser{
    private:
        Confs* ctrl;
        string name;
        
        ConfAliaser &add(string tname, vector<string> possibleNames){
            ctrl->aliases[name].optionalNamesInEachProviders[tname] = possibleNames;
    
            ctrl->informPotentialVariables(tname, possibleNames);
    
            return *this;            
        }
    public:
    
        //!!use the createAlias method of Confs class to create an instance of this class!!
        ConfAliaser(Confs *ctrl, string aliasName):ctrl(ctrl), name(aliasName){
            if (!ctrl->aliases.count(aliasName))
                ctrl->aliases[aliasName] = ConfAliasInfo();
        }

        /// @brief same of forProvider<T>. specifies how this configuration is called in a particular provider (Add a name for this configuration in a specific provider()
        /// @tparam T type of the provider
        /// @param possibleNames possible names of this configuration in the specified provider (T)
        /// @return return a reference to this object
        template<class T>
        ConfAliaser &add(vector<string> possibleNames){
            auto tname = typeid(T).name();
            return this->add(tname, possibleNames);
        }

        /// @brief same of add<T>. specifies how this configuration is called in a particular provider (Add a name for this configuration in a specific provider()
        /// @tparam T type of the provider
        /// @param possibleNames possible names of this configuration in the specified provider (T)
        /// @return return a reference to this object
        template<class T>
        ConfAliaser &forProvider(vector<string> possibleNames){
            return this->add<T>(possibleNames);
        }

        /// @brief Specifies how this configuration is called in any provider (for each provider, calls de 'add/forProvider' function)
        /// @param possibleNames possible names of this configuration in the providers
        /// @return 
        ConfAliaser &addForAnyProvider(vector<string> possibleNames)
        {
            for (auto &c: ctrl->providers)
            {
                string idName = c->getTypeIdName();
                this->add(idName, possibleNames);
            }

            return *this;
        }

        // Sets the default value of this configuration. When the configuration is requestes but not found in any provider, this values will be returned
        ConfAliaser &setDefaultValue(DynamicVar defaultValue){
            ctrl->aliases[name].defaultValue = defaultValue;
            return *this;
        }

    };

    class ConfPlaceHolder{
    private:
        Confs* ctrl;
    public:
        ConfPlaceHolder(Confs *ctrl);
        ConfPlaceHolder& add(string replace, string by);
        ConfPlaceHolder& add(vector<tuple<string, string>> replacesAndBys);
    };

public:

    //Creates a Conf instance with no providers. You must add providers to make a useful configuration system. 
    //If you do not add any provider, the configurations will be just a runtime, in memory and volatile configuration system.
    //you can use Confs without providers for testing purposes, but it is not useful for production.
    //use the addProvider method or other constructor to specify providers.
    Confs(){};
    Confs(vector<IConfProvider*> initialProviders);
    ~Confs();

    //Warning! Providers will be automatically deleted whend the Confs isntance be destroyed
    void addProvider(IConfProvider *provider);

    //return a ConfAliaser object, that can be used to construct an alias. The alias allow you to identify diferent names for same configuration.
    ConfAliaser createAlias(string aliasName);

    ConfPlaceHolder createPlaceHolders();

    //identify the alias and scrolls over all providers. The first one with the configuration will be used to return the value.
    //
    //The defaultValue will be returned if it is not empty and no provider has the configuration.
    //If defaultValue is empty and no provider has the configuration, the function will return the default value setted
    //in the definition of the alias. It allow you tu use a diferent default value for each alias, but it is not recomended, because
    //it can lead to incorrent behavior in different parts of the system. If you are not sure about the default value, just let it empty.
    DynamicVar getA(string alias, DynamicVar defaultValue = "");

    void listenA(string alias, function<void(DynamicVar)> f, bool callFImedially = true, DynamicVar defaultValueForImediateFCall = "");
    string applyPlaceHolders(string source);

    vector<tuple<string, DynamicVar>> getAllConfigurationsA();

};

#endif