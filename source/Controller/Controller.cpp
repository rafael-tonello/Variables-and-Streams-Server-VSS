#include "Controller.h"

namespace HomeAut { namespace Controller {
    Controller::Controller(){
        Promise::initLib(&this->tasker);
    }

    VarNode* Controller::_findNode(string name, VarNode* curr, bool createNewNodes)
    {
        VarNode* ret = NULL;


        string childNodeName;
        string remainingName;
        //separes the name in current node name and remaing names to be parsed
        if (name.find('.') != string::npos)
        {
            childNodeName = name.substr(0, name.find('.'));
            remainingName = name.substr(name.find('.') + 1);
        }
        else
        {
            childNodeName = name;
            remainingName = "";
        }

        //checks if curr contains or not a child named 'newNodeName'
        if (curr->childs.count(childNodeName) == 0)
        {
            if (createNewNodes)
            {
                //create a child
                VarNode tmp;
                tmp.name = childNodeName;
                tmp.parent = curr;
                tmp.fullName = curr->fullName + (curr->fullName != "" ? "." : "") + childNodeName;
                curr->childs[childNodeName] = tmp;
            }
            else
                return NULL;

        }

        VarNode* next = &curr->childs[childNodeName];

        //checks if search reaches the end
        if (remainingName == "")
            return next;
        else
            return _findNode(remainingName, next);
    }

    string Controller::_resolveVarName(string aliasOrVarName)
    {
        VarNode* dest = _findNode(aliasOrVarName, &rootNode, false);
        if ((dest != NULL) && (dest->type == VarType::Alias))
            return _resolveVarName(dest->value.getString());
        else
            return aliasOrVarName;
        
    }

    //creates or change a variable
    future<void> Controller::setVar(string name, DynamicVar value)
    {
        return tasker.enqueue([name, value, this](){
            
            //resolve the destination varname
            string rname = _resolveVarName(name);

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
            node->valueSetted = true;


            //notify observers
            //auto th = tasker.enqueue([this, name, value, node](){
                for (auto f : node->observers)
                {
                    pendingTasks.push_back(
                        tasker.enqueue([rname, value](VarObserverInfo f2){
                            f2.fun(rname, value, f2.args, f2.id);
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
                                    func2.fun(rname, value, func2.args, func2.id);
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
        return tasker.enqueue([&](){
            //set the value of the var with the name of the destination variable. Uses setVar func to do this.
            setVar(name, DynamicVar(string(dest))).get();

            //get the node
            auto node = _findNode(name, &rootNode);

            //set the type of the node to 'alias'
            node->type = VarType::Alias;
        });
    }

    //returned the literal value (a variable name) of an alias
    string Controller::getAliasValue(string aliasName)
    {
        
    }

    //start to observate a variable
    string Controller::observeVar(string varName, observerCallback callback, void* args, string id)
    {
        varName =  _resolveVarName(varName);

        //creates an id (if it was not specified)
        if (id == "")
            id = _createId();

        VarNode* node = _findNode(varName, &rootNode);

        //checks if observer is already in the list
        bool found = false;
        for (int c = 0; c < node->observers.size(); c++)
        {
            if(node->observers[c].id == id)
            {
                found = true;
                //update the observer
                node->observers[c].fun = callback;
                node->observers[c].args = args;
            }
        }

        if (!found)
        {
            VarObserverInfo observerInfo;
            observerInfo.args = args;
            observerInfo.fun = callback;
            observerInfo.id = id;

            varsObserversMutex.lock();
            node->observers.push_back(observerInfo);
            varsObserversMutex.unlock();

            observersShorcut[id] = node;
        }

        //call the observers wit current var value
        tasker.enqueue([node, args, id, callback, this, varName](){
            

            if (node->name == "*")
            {
                //notify observer with all values of node->parent and their childs
                function<void(VarNode* tmpNode)> notifierFunc;
                notifierFunc = [&, this](VarNode *tmpNode){
                    if (tmpNode != NULL)
                    {
                        if (tmpNode->valueSetted)
                        {
                            tasker.enqueue([&](){
                                //to notify the observer, first get the variable value using the 'getVar' function. This is necessary why there are some
                                //operations (like load values from files on startyp) that are made by getVar function.
                                auto value = std::get<1>(getVar(tmpNode->fullName, DynamicVar(string(""))).get()[0]);
                                callback(tmpNode->name, value, args, id);
                            });
                        }

                        for (auto tmpChild: tmpNode->childs)
                        {
                            VarNode* tmp = &tmpChild.second;
                            notifierFunc(tmp);
                        }
                    }
                    return;
                };
                notifierFunc(node->parent);
            }
            else
            {
                //to notify the observer, first get the variable value using the 'getVar' function. This is necessary why there are some
                //operations (like load values from files on startyp) that are made by getVar function.
                auto value = std::get<1>(getVar(varName, DynamicVar(string(""))).get()[0]);
                callback(node->name, value, args, id);
            }
        });

        return id;
    }

    //stop observate variable
    void Controller::stopObservingVar(string id)
    {
        if (observersShorcut.count(id) > 0)
        {
            VarNode *node = observersShorcut[id];
            //locate the observer in the node and remove it
            for (int c = node->observers.size(); c >=0 ; c--)
            {
                if (node->observers[c].id == id)
                {
                    node->observers.erase(node->observers.begin() + c);
                    //return;
                }
            }
        }
    }

    string Controller::_createId()
    {
        //TODO: Create an unique ID
    }

    future<vector<tuple<string, DynamicVar>>> Controller::getVar(string name, DynamicVar defaultValue)
    {

    }

    future<void> Controller::delVar(string varname)
    {

    }

    future<vector<string>> Controller::getChildsOfVar(string parentName)
    {

    }

}}
