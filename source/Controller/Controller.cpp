#include "Controller.h"

namespace HomeAut { namespace Controller {
    Controller::Controller(){
        Promise::initLib(&this->tasker);
    }

    string Controller::resolveVarName(string aliasOrVarName)
    {
        
    }

    VarNode* Controller::_findNode(string name, VarNode* curr)
{
    VarNode* ret = NULL;

    string currName;
    if (name.find('.') != string::npos)
    {
        currName = name.substr(0, name.find('.'));
        name = name.substr(name.find('.') + 1);
    }
    else
    {
        currName = name;
        name = "";
    }

    //checks if curr contains or not a child named 'currName'
    if (curr->childs.count(currName) == 0)
    {
        //create a child
        VarNode tmp;
        tmp.name = currName;
        tmp.parent = curr;
        curr->childs[currName] = tmp;
    }

    VarNode* next = &curr->childs[currName];

    //checks if search reaches the end
    if (name == "")
        return next;
    else
        return _findNode(name, next);
}



//creates or change a variable
future<void> Controller::setVar(string name, DynamicVar value)
{
    return tasker.enqueue([name, value, this](){
        
        //resolve the destination varname
        string rname = resolveVarName(name);

        vector<future<void>> pendingTasks;

        if (name.find('*') != string::npos)
        {
            cerr << "Invalid setVar parameter. A variable with '*' can't be setted";
            //throw "Invalid setVar parameter. A variable with '*' can't be setted";
            return;
        }

        //set the variable
        VarNode* node = _findNode(rname, &rootNode);
        node->value = value;


        //notify observers
        //auto th = tasker.enqueue([this, name, value, node](){
            for (auto f : node->observers)
            {
                pendingTasks.push_back(
                    tasker.enqueue([rname, value](VarObserverInfo f2){
                        f2.fun(rname, value, f2.args);
                    }, f)
                );
            }

            //after notify directly observers, finds (in the same dictionary) observers using regular expressions
            VarNode* tmpNode = node;
            while (tmpNode != NULL)
            {
                //checks if node has a special child name '*'
                if (tmpNode->childs.count("*") > 0)
                {
                    for (auto func : tmpNode->childs["*"].observers)
                    {
                        pendingTasks.push_back(
                            tasker.enqueue([rname, value](VarObserverInfo func2){
                                func2.fun(rname, value, func2.args);
                            }, func)
                        );
                    }
                }
                tmpNode = tmpNode->parent;
            }
            //http://www.cplusplus.com/reference/regex/
        //});

        //th.wait();

        //wait all pending tasks
        for (auto &c: pendingTasks)
            c.wait();

        cout << "all pending taskas was done" << endl;
    });
}

    //create an alias (a shortcut) to a variable (or to another alias)
    future<void> Controller::createAlias(string name, string dest)
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