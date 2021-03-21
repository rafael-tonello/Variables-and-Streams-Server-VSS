#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <future>
#include <map>

#include "./Shared/Misc/DynamicVar.h"
#include "./Shared/ThreadPool/ThreadPool.h"

using namespace std;
using namespace Shared;

struct VarObserverInfo{
    string id = "";
    void* args;
    function<void(string name, DynamicVar value, void* args)> fun;
};

struct VarNode{
    VarNode* parent = NULL;
    string name = "";
    DynamicVar value;
    map<string, VarNode> childs;
    vector<VarObserverInfo> observers;
};

class Controller{   
private:
    VarNode rootNode;
    ThreadPool tasker;


    VarNode* _findNode(string name, VarNode* curr);
public:
    future<void>  setVar(string name, DynamicVar value);

};



void controller_setvar_test();