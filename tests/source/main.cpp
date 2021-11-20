#include <iostream>
#include<vector>
#include<string>
#include <map>
#include <functional>

#include "tester.h"

#include "./Services/APIs/PHOMAU/SocketInfo.test.h"
#include "./Services/APIs/PHOMAU/PHOMAU.test.h"
#include "./Controller/Controller.test.h"
#include "./Shared/DependencyInjectionManager/DependencyInjectionManager.test.h"
#include "./Shared/Confs/internal/SimpleConfFileProvider.test.h"
#include "./Shared/Confs/Confs.test.h"

using namespace std;
int main(int argc, char* argv[]){
    
    vector<Tester*> testers;

    //***** testers instances
    //***** make your changes only here
        testers.push_back(new SocketInfoTester());
        testers.push_back(new PhomauTester());
        testers.push_back(new ControllerTester());
        testers.push_back(new DependencyInjectionManagerTester());
        testers.push_back(new SimpleConfFileProviderTester());
        testers.push_back(new ConfsTester());
    //*****

    return Tester::runTests(testers, argc, argv);
}