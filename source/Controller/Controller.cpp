#include "Controller.h"

namespace HomeAut { namespace Controller {
    Controller::Controller(){
        Promise::initLib(&this->tasker);
    }

    string Controller::resolveVarName(string aliasOrVarName)
    {
        
    }

    //creates or change a variable
    void Controller::setVar(string name, DynamicVar variable)
    {

        //set the variable

        //notify observers
        tasker.enqueue([this, name, variable](){
            if (this->varsObservers.count(name) > 0)
            {
                for (auto f : this->varsObservers[name])
                {
                    tasker.enqueue([name, variable](VarObserverInfo f2){
                        f2.fun(name, variable, f2.args);
                    }, f);
                }
            }

            //after notify directly observers, finds (in the same dictionary) observers using regular expressions
            //http://www.cplusplus.com/reference/regex/
        });
        
    }

    //create an alias (a shortcut) to a variable (or to another alias)
    void Controller::setAlias(string aliasName, string varName)
    {
        
    }

    //returned the literal value (a variable name) of an alias
    string Controller::getAliasValue(string aliasName)
    {
        
    }

    //start to observate a variable
    void Controller::observeVar(string varName, function<void(string name, string value, void* args)> callback, void* args = NULL, string id = "")
    {
        varsObserversMutex.lock();
        //checks if maps already contains the 'varName' position
        if (this->varsObservers.count(varName) == 0)
        {
            vector<VarObserverInfo> t;
            this->varsObservers[varName] = t;
        }
        
        //prepare the VarObserverInfo to be added to map
        VarObserverInfo v;
        v.args = args;
        v.fun = callback;
        v.id = id;

        //add the VarObserverInfo to the map
        this->varsObservers[varName].push_back(v);
        varsObserversMutex.unlock();
    }

    //stop observate variable
    void Controller::stopObserve(string id)
    {
        varsObserversMutex.lock();
        //scrolls over the map
        for (auto &c: varsObservers)
        {
            //for each map psotion, scrolls its vector of VarObserverInfo
            for (int c2 = c.second.size(); c2 >= 0; c2++)
            {
                //if the current VarObserverInfo contains same id of 'id' argument, delete this
                if (c.second[c2].id == id)
                {
                    c.second.erase(c.second.begin() + c2);
                }
            }
        }

        varsObserversMutex.unlock();
    }
}}