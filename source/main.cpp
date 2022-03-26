#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <signal.h>
#include <unistd.h>
#include <limits.h>


#include <Controller.h>
#include "Services/APIs/PHOMAU/PHOMAU.h"
#include <dependencyInjectionManager.h>
#include <Confs.h>
#include <StorageInterface.h>
#include <Storage/VarSystemLib/VarSystemLibStorage.h>
#include <SysLink.h>
#include <logger.h>
#include <logger/writers/LoggerConsoleWriter.h>

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
    SysLink sl;
    string confFile = "/etc/vss/confs.conf";
    if (sl.fileExists(getApplicationDirectory() + "/confs.confs"))
        confFile = getApplicationDirectory() + "/confs.confs";

    
    
    DependencyInjectionManager dim;

    
    
    Shared::Config *conf = new Shared::Config(confFile);
    conf->createPlaceHolder("%PROJECT_DIR%", getApplicationDirectory()  + "/");

    dim.addSingleton<ILogger>(new Logger({new LoggerConsoleWriter(true)}));
    dim.addSingleton<ThreadPool>(new ThreadPool(4));
    dim.addSingleton<Shared::Config>(conf, {typeid(Shared::Config).name()});
    dim.addSingleton<StorageInterface>(new VarSystemLibStorage(&dim));
    /*two points to controller (to allow systems to find it by all it types):
     the controller can be find by use of get<TheController> and get<ApiMediatorInterface>*/
    dim.addSingleton<TheController>(new TheController(&dim), {typeid(TheController).name(), typeid(ApiMediatorInterface).name()});
    dim.addSingleton<PHOMAU>(new PHOMAU(5021, dim.get<ApiMediatorInterface>(), dim.get<ILogger>()));


    auto logger = dim.get<ILogger>();

    /* #region Initial information messages */
        logger->info("", "the system is running");
        logger->info("", "\tPHOMAU API port: 5021");
    /* #endregion */

    //prevent program close (equivalent to while (true))
    exitMutex.lock();
    exitMutex.lock();

    /* #region finalization of the program */
        logger->info("", "The program is exiting. Please wait...");
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