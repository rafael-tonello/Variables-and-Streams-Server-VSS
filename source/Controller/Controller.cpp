#include "Controller.h"

using namespace Controller;
TheController::TheController(){
    
}

TheController::~TheController(){
    
}

VarNode* TheController::_findNode(string name, VarNode* curr, bool createNewNodes)
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

    //if the node is an alias, continue the search in the destination
    //to remove this feature and allow childs for aliases, just comment these two lines
    if (next->type == VarType::Alias)
        next = _findNode(next->value.getString(), &rootNode);

    //checks if search reaches the end
    if (remainingName == "")
        return next;
    else
        return _findNode(remainingName, next);
}

ResolveVarNameResult TheController::_resolveVarName(string aliasOrVarName)
{
    VarNode* dest = _findNode(aliasOrVarName, &rootNode, false);
    ResolveVarNameResult result;
    result.search = aliasOrVarName;
    result.searchIsAnAlias = false;



    if ((dest != NULL) && (dest->type == VarType::Alias))
    {
        result.destination = _resolveVarName(dest->value.getString()).destination;
        result.searchIsAnAlias = true;
    }
    else
        result.destination = aliasOrVarName;

    return result;
    
}

//creates or change a variable
future<void> TheController::setVar(string name, DynamicVar value)
{
    //check if name isn't a internal flag var
    if (name.find('_') == 0 || name.find("._") != string::npos)
    {
        cerr << "variabls started with underscorn (_) are just for internal flags and can't be setted by clients" << endl;
        return;
    }

    //checks if the variabel is locked
    if (this->getVarInternalFlag(name, "_lock", 0).getInt() == 1)
    {
        cerr << "The variable '"<< name << "' is locked and can't be changed by setVar" << endl;
        return;
    }

    return this->internalSetVar(name, value);

}
future<void> TheController::internalSetVar(string name, DynamicVar value)
{
    return tasker.enqueue([name, value, this](){
        
        //resolve the destination varname
        string rname = _resolveVarName(name).destination;

        vector<future<void>> pendingTasks;

        if (name.find('*') != string::npos)
        {
            #ifdef __TESTING__
                Tester::global_test_result = "Invalid setVar parameter. A variable with '*' can't be setted";
            #endif
            cerr << "Invalid setVar parameter. A variable with '*' can't be setted" << endl;
            //throw "Invalid setVar parameter. A variable with '*' can't be setted";
            return;
        }

        //find the node of the variable
        VarNode* node = _findNode(rname, &rootNode);

        //checks if the value is newer
        if (node->value.isEquals(&value))
            return;

        //set the variable
        node->value = value;
        node->valueSetted = true;

        #ifdef __TESTING__
            Tester::global_test_result = "Setted node: "+to_string((uint64_t)(node))+", setted value: "+(const_cast <DynamicVar&>(value)).getString();
        #endif


        //notify observers
        //auto th = tasker.enqueue([this, name, value, node](){
            for (auto f : node->observers)
            {
                pendingTasks.push_back(
                    tasker.enqueue([rname, value](VarObserverInfo f2){
                        f2.fun(f2.aliasName != "" ? f2.aliasName : rname, value, f2.args, f2.id);
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
                                func2.fun(func2.aliasName != "" ? func2.aliasName : rname, value, func2.args, func2.id);
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

        //cout << "all pending taskas was done" << endl;
    });
}

future<void> TheController::lockVar(string varName)
{
    //ir variable if currently locked, add an observer to it "._lock" property and wait the change of this to 0

    //set the vars property'a '._lock' to 1 (use setVar to change the ._lock)

    //set var can't change a lokced var

}

future<void> TheController::unlockVar(string varName)
{

}

//create an alias (a shortcut) to a variable (or to another alias)
future<void> TheController::createAlias(string name, string dest)
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
future<string> TheController::getAliasValue(string aliasName)
{
    
}

future<void> TheController::deleteAlias(string aliasName)
{
    
}

//start to observate a variable
string TheController::observeVar(string varName, observerCallback callback, void* args, string id)
{
    ResolveVarNameResult resolveVarNameResult = _resolveVarName(varName);

    //creates an id (if it was not specified)
    if (id == "")
        id = _createId();

    VarNode* node = _findNode(resolveVarNameResult.destination, &rootNode);

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
            node->observers[c].aliasName = resolveVarNameResult.searchIsAnAlias ? resolveVarNameResult.search : "";

        }
    }

    if (!found)
    {
        VarObserverInfo observerInfo;
        observerInfo.args = args;
        observerInfo.fun = callback;
        observerInfo.id = id;
        observerInfo.aliasName = resolveVarNameResult.searchIsAnAlias ? resolveVarNameResult.search : "";

        varsObserversMutex.lock();
        node->observers.push_back(observerInfo);
        varsObserversMutex.unlock();

        observersShorcut[id] = node;
    }

    //call the observers wit current var value
    tasker.enqueue([node, args, id, callback, this, varName, resolveVarNameResult](){
        

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
                            callback(tmpNode->fullName, value, args, id);
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
            callback(varName, value, args, id);
        }
    });

    return id;
}

//stop observate variable
void TheController::stopObservingVar(string id)
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

string TheController::_createId()
{
    //TODO: Create an unique ID
}

future<vector<tuple<string, DynamicVar>>> TheController::getVar(string name, DynamicVar defaultValue)
{

}

future<void> TheController::delVar(string varname)
{

}

future<vector<string>> TheController::getChildsOfVar(string parentName)
{

}


DynamicVar TheController::getVarInternalFlag(string vName, string flagName, DynamicVar defaultValue)
{
    if (flagName != "")
    {
        if (flagName[0] != '_');
            flagName = "_"+flagName;
        
        auto flagValue = this->getVar(vName + "."+flagName, defaultValue).get();
        return std::get<1>(flagValue[0]);
    }
}

void TheController::setVarInternalFlag(string vName, string flagName, DynamicVar value)
{
    if (flagName != "")
    {
        if (flagName[0] != '_');
            flagName = "_"+flagName;
        
        this->internalSetVar(vName + "."+flagName, value).get();
    }
}