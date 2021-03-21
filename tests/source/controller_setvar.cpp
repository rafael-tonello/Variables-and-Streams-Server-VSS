#include "controller_setvar.h"

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
    {
        cout << "Chegou no fim da busca" << endl;
        return next;
    }
    else
        return _findNode(name, next);
}



//creates or change a variable
future<void> Controller::setVar(string name, DynamicVar value)
{
    return tasker.enqueue([name, value, this](){

        vector<future<void>> pendingTasks;

        if (name.find('*') != string::npos)
        {
            cerr << "Invalid setVar parameter. A variable with '*' can't be setted";
            //throw "Invalid setVar parameter. A variable with '*' can't be setted";
            return;
        }

        //set the variable
        VarNode* node = _findNode(name, &rootNode);
        node->value = value;


        //notify observers
        //auto th = tasker.enqueue([this, name, value, node](){
            for (auto f : node->observers)
            {
                pendingTasks.push_back(
                    tasker.enqueue([name, value](VarObserverInfo f2){
                        f2.fun(name, value, f2.args);
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
                            tasker.enqueue([name, value](VarObserverInfo func2){
                                func2.fun(name, value, func2.args);
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

void controller_setvar_test(){
    Controller controller;
    auto op1 = controller.setVar("v1.v2.v3", DynamicVar(string("foo3")));
    auto op2 = controller.setVar("v1.v2.v3.v4", DynamicVar(string("foo4")));
    auto op3 = controller.setVar("v1", DynamicVar(string("foo1")));
    auto op4 = controller.setVar("v1.v2", DynamicVar(string("foo2")));

    op1.wait();
    op2.wait();
    op3.wait();
    op4.wait();




    cout << "Ok, op was done!!" << endl;
    op4 = controller.setVar("v1.v2", DynamicVar("foo2"));

    int b = 10;





}