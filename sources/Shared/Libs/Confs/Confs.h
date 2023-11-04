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
 *      Lets supose your system needs a port for a socket conneciton. Ans also lets supose this port can be configured in a configuration file, in a 
 *      environmnet variable and even can be informed in the command line of you program. 
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
 */


using namespace std;
struct ConfAliasInfo{
    //map<providername, possible_names_of_the_conf_in_this_provider>
    map<string, vector<string>> optionalNamesInEachProviders;
    vector<function<void(DynamicVar)>> listeners = {};
    DynamicVar lastValidValue;
};

class Confs{
private:
    map<string, ConfAliasInfo> aliases;
    vector<IConfProvider*> providers;
    vector<tuple<string, string>> placeHolders;

    void confChangedInAProvider(string providerName, string name, DynamicVar value);

    class ConfAliaser{
    private:
        Confs* ctrl;
        string name;
    public:
        ConfAliaser(Confs *ctrl, string aliasName):ctrl(ctrl), name(aliasName){
            if (!ctrl->aliases.count(aliasName))
                ctrl->aliases[aliasName] = ConfAliasInfo();
        }

        ConfAliaser &add(string tname, vector<string> possibleNames){
            ctrl->aliases[name].optionalNamesInEachProviders[tname] = possibleNames;

            return *this;            
        }

        template<class T>
        ConfAliaser &add(vector<string> possibleNames){
            auto tname = typeid(T).name();
            return this->add(tname, possibleNames);
        }

        template<class T>
        ConfAliaser &forProvider(vector<string> possibleNames){
            return this->add<T>(possibleNames);
        }

        ConfAliaser &addForAnyProvider(vector<string> possibleNames)
        {
            for (auto &c: ctrl->providers)
            {
                string idName = c->getTypeIdName();
                this->add(idName, possibleNames);
            }

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

    Confs(){};
    Confs(vector<IConfProvider*> initialProviders);
    ~Confs();

    //Warning! Providers will be automatically deleted whend the Confs isntance be destroyed
    void addProvider(IConfProvider *provider);

    //return a ConfAliaser object, that can be used to construct an alias. The alias allow you to identify diferent names for same configuration.
    ConfAliaser createAlias(string aliasName);

    ConfPlaceHolder createPlaceHolders();

    //identify the alias and scrolls over all providers. The first one with the configuration will be used to return the value.
    DynamicVar getA(string alias, DynamicVar defaultValue = "");

    void listenA(string alias, function<void(DynamicVar)> f, bool callFImedially = true, DynamicVar defaultValueForImediateFCall = "");
    string applyPlaceHolders(string source);

};

#endif