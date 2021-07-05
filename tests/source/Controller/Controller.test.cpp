#include "Controller.test.h"
vector<string> ControllerTester::getContexts()
{
    return {"Controller"};
}

void ControllerTester::run(string context)
{
    if (context == "Controller")
    {
        this->testSetVar();
        this->testGetVar();
        /*
        future<void> setVar(string name, DynamicVar value);
        future<vector<tuple<string, DynamicVar>>> getVar(string name, DynamicVar defaultValue);
        future<void> delVar(string varname);
        VarNode* _findNode(string name, VarNode* curr, bool createNewNodes = true);
        string _resolveVarName(string aliasOrVarName);
        string _createId();
        map<string, VarNode*> observersShorcut;
        
        string getAliasValue(string aliasName);
        future<void> createAlias(string name, string dest);
        string observeVar(string varName, observerCallback callback, void* args = NULL, string observerId = "");
        void stopObservingVar(string observerId);
        future<vector<string>> getChildsOfVar(string parentName);
        */

    }
}

void ControllerTester::testSetVar()
{
    //set a var
    this->test("Controller::setVar(tests.test1, test1value) is valid varset", [&](){
        this->ctrl.setVar("tests.test1", DynamicVar(string("test1value"))).wait();

        string expected = "Setted node: [a valid address], setted value: test1value";

        /*string addrs = Tester::global_test_result.substr(Tester::global_test_result.find(':') + 2);
        addrs = addrs.substr(0, addrs.find(','));
        uint64_t addri;
        std::istringstream iss(addrs);
        iss >> addri;
        VarNode * node = (VarNode*)(addri);*/

        return TestResult{
            Tester::global_test_result.find("setted value: test1value") != string::npos,
            expected,
            Tester::global_test_result
        };
    });

    //try set a var with *
    this->test("Controller:setVar(tests.test2.*, test2value) is invalid", [&](){
        this->ctrl.setVar("tests.test2.*", DynamicVar(string("test2value"))).wait();

        string expected = "Invalid setVar parameter. A variable with '*' can't be setted";
        return TestResult{
            Tester::global_test_result == expected,
            expected,
            Tester::global_test_result
        };
    });

    //verify the variable tree structure
    this->test("Controller: Validation of var tree structure", [&](){

        function<bool(VarNode*, string)> checkStructure;
        checkStructure = [&](VarNode* curr, string ONName){
            string currName = ONName.find('.') != string::npos ? ONName.substr(0, ONName.find('.')) : ONName;
            string remainName = ONName.find('.') != string::npos ? ONName.substr(ONName.find('.')+1) : "";

            if (curr->childs.count(currName) > 0)
            {
                if (remainName != "")
                    return checkStructure(&curr->childs[currName], remainName);
                else
                    return true;

            }
            else
                return false;
        };

        vector<future<void>> sets;
        sets.push_back(this->ctrl.setVar("tests.test1", DynamicVar(string("v"))));
        sets.push_back(this->ctrl.setVar("tests.test2.test21.test211", DynamicVar(string("v"))));
        sets.push_back(this->ctrl.setVar("tests.test2", DynamicVar(string("v"))));
        sets.push_back(this->ctrl.setVar("tests.test1.test11", DynamicVar(string("v"))));
        sets.push_back(this->ctrl.setVar("tests.test1.test12.test121", DynamicVar(string("v"))));
        sets.push_back(this->ctrl.setVar("tests.test2.test22.test221", DynamicVar(string("v"))));
        sets.push_back(this->ctrl.setVar("test2.test1.test22.test221.test2211", DynamicVar(string("v"))));

        for(auto &c: sets) c.wait();

        bool result = true;


        result &= checkStructure(&this->ctrl.rootNode, "tests.test1.test11");
        result &= checkStructure(&this->ctrl.rootNode, "tests.test1.test12.test121");
        result &= checkStructure(&this->ctrl.rootNode, "tests.test2.test21.test211");
        result &= checkStructure(&this->ctrl.rootNode, "tests.test2.test22.test221");
        result &= checkStructure(&this->ctrl.rootNode, "test2.test1.test22.test221.test2211");
        
        return result;
    });

}

void ControllerTester::testGetVar()
{
    //first of all, inject some data int the controller class (it can be done using setVar function or direct inject data)

    //get an existent var

    //get an inexistent var

    //get a var using '*'

}