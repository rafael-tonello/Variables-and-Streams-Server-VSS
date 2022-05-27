#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <functional>

#include "tester.h"

#include "./Services/APIs/PHOMAU/SocketInfo.test.h"
#include "./Services/APIs/PHOMAU/PHOMAU.test.h"
#include "./Controller/Controller.test.h"
#include "./Shared/Libs/DependencyInjectionManager/DependencyInjectionManager.test.h"
#include "./Shared/Libs/Confs/internal/SimpleConfFileProvider.test.h"
#include "./Shared/Libs/Confs/Confs.test.h"
#include "./Controller/Internal/Controller_ClientHelper.test.h"
#include "./Controller/Internal/Controller_VarHelper.test.h"

using namespace std;
int main(int argc, char* argv[]){

    DynamicVar a("teste");
    cout << "Teste valor: " << a.getString() << endl;
    
    vector<Tester*> testers;

    //***** testers instances
    //***** make your changes only here
        testers.push_back(new SocketInfoTester());
        testers.push_back(new PhomauTester());
        testers.push_back(new ControllerTester());
        testers.push_back(new DependencyInjectionManagerTester());
        testers.push_back(new SimpleConfFileProviderTester());
        testers.push_back(new ConfsTester());
        testers.push_back(new Controller_ClientHelperTester());
        testers.push_back(new Controller_VarHelperTester());

        //logger
        //  LoggerFileWriter
        //  LoggerLambdaWriter
        //  LoggerConsoleWriter
        //Confs
        //Controller
        //Controller_ClientHelper
        //Controller_VarHelper
        //FileVars
        //SysLink
        //ThreadPool
        //DynamicVar
        //Observable
        //TaggedObject
        //Utils

    //*****

    return Tester::runTests(testers, argc, argv);
}