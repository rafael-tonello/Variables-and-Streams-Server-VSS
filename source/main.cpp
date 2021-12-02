#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <signal.h>
#include <unistd.h>
#include <limits.h>


#include "Controller/Controller.h"
#include "Services/APIs/PHOMAU/PHOMAU.h"
#include "Shared/DependencyInjectionManager/dependencyInjectionManager.h"
#include "Shared/Confs/Confs.h"
#include "Services/VarSystem/SysLink.h"

using namespace std;
using namespace Controller;
using namespace API;

mutex exitMutex;

void signalHandler( int signum ) {
   cout << "Interrupt signal (" << signum << ") received.\n";
   exitMutex.unlock();
}

std::string getApplicationDirectory();

int main(){
    
    signal(SIGINT, signalHandler);  
    signal(SIGQUIT, signalHandler);  
    signal(SIGABRT, signalHandler);  
    signal(SIGSEGV, signalHandler);  
    signal(SIGTERM, signalHandler);  
    signal(SIGKILL, signalHandler);  
    

    //determine the configuration file
    string confFile = "/etc/vss/confs.conf";
    SysLink sl;
    if (sl.fileExists(getApplicationDirectory() + "/confs.confs"))
        confFile = getApplicationDirectory() + "/confs.confs";




    
    /* #region code without dependency injection manager */
        /*//the application main controller
        TheController* controller = new TheController();

        // #region the other services and dependency injection
            PHOMAU* phomau = new PHOMAU((int)5021, controller);
        // #endregion*/
    /* #endregion */
    
    /* #region code with dependency injection manager */
        DependencyInjectionManager dim;

        
        //two points to controller (to allow systems to find it by all it types):
        // the controller can be find by use of get<TheController> and get<ApiMediatorInterface>
        Shared::Config *conf = new Shared::Config(confFile);
        conf->createPlaceHolder("%PROJECT_DIR%", getApplicationDirectory()  + "/");

        dim.addSingleton<Shared::Config>(conf, {typeid(Shared::Config).name()});

        dim.addSingleton<TheController>(new TheController(&dim), {typeid(TheController).name(), typeid(ApiMediatorInterface).name()});
        dim.addSingleton<PHOMAU>(new PHOMAU(5021, dim.get<ApiMediatorInterface>()));

    /* #endregion */
        

    /* #region Initial information messages */
        cout << "the system is running" << endl;
        cout << "\tPHOMAU API port: 5021" << endl;
    /* #endregion */

    //prevent program close (equivalent to while (true))
    exitMutex.lock();
    exitMutex.lock();

    /* #region finalization of the program */
        cout << "The program is exiting. Please wait..." <<endl;
        //delete controller;
        //delete phomau;
        delete dim.get<PHOMAU>();
        delete dim.get<TheController>();
    /* #endregion */

    exit(0);  
    return 0;
}

std::string getApplicationDirectory() 
{
    char result[ PATH_MAX ];
    ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    std::string appPath = std::string( result, (count > 0) ? count : 0 );

    std::size_t found = appPath.find_last_of("/\\");
    return appPath.substr(0,found);
}