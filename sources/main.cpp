#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <signal.h>
#include <unistd.h>
#include <limits.h>


#include <Controller.h>
#include "Services/APIs/VSTP/VSTP.h"
#include <httpapi.h>
#include <dependencyInjectionManager.h>
#include <Confs.h>
#include <StorageInterface.h>
#include <Storage/VarSystemLib/VarSystemLibStorage.h>
#include <VarSystem/SysLink.h>
#include <logger.h>
#include <LoggerConsoleWriter.h>
#include <LoggerFileWriter.h>
#include <ServerDiscovery.h>

#include <messagebus.h>

using namespace std;
using namespace Controller;
using namespace API;

mutex exitMutex;
int exitSignal = -1;

void signalHandler( int signum ) {
    exitSignal = signum;
   cout << "Interrupt signal (" << signum << ") received.\n";
   exitMutex.unlock();
}

std::string getApplicationDirectory();
std::string findConfigurationFile();
std::string determinteLogFile();

bool isRunningInPortableMode();
void handleSignals();

//semantic versioning
string INFO_VERSION = "1.8.2";

int main(){
    

    handleSignals();  
    
    DependencyInjectionManager dim;
    
    Shared::Config *conf = new Shared::Config(findConfigurationFile());
    conf->createPlaceHolder("%PROJECT_DIR%", getApplicationDirectory());

    dim.addSingleton<ILogger>(new Logger({new LoggerConsoleWriter(), new LoggerFileWriter(determinteLogFile())}, true));
    dim.addSingleton<ThreadPool>(new ThreadPool(4));
    dim.addSingleton<MessageBus<JsonMaker::JSON>>(new MessageBus<JsonMaker::JSON>(dim.get<ThreadPool>(), [](JsonMaker::JSON &item){return item.getChildsNames("").size() == 0;}));

    dim.addSingleton<Shared::Config>(conf, {typeid(Shared::Config).name()});
    dim.addSingleton<StorageInterface>(new VarSystemLibStorage(&dim));
    /*two points to controller (to allow systems to find it by all it types):
     the controller can be find by use of get<TheController> and get<ApiMediatorInterface>*/
    dim.addSingleton<TheController>(new TheController(&dim), {typeid(TheController).name(), typeid(ApiMediatorInterface).name()});
    dim.addSingleton<VSTP>(new VSTP(5021, dim));
    dim.addSingleton<API::HTTP::HttpAPI>(new API::HTTP::HttpAPI(5023, &dim));
    dim.addSingleton<ServerDiscovery>(new ServerDiscovery(dim, INFO_VERSION));


    auto logger = dim.get<ILogger>();

    /* #region Initial information messages */
        logger->info("", 
            string("The VSS has been started\n") +
            string("+-- Version: "+ INFO_VERSION + "\n")+
            string("+-- Portable mode: ")+ string(isRunningInPortableMode() ? "Yes":"No")+ "\n" + 
            string("|   +-- conf file: "+findConfigurationFile()+"\n")+
            string("|   +-- log file: "+determinteLogFile()+"\n")+
            string("|   +-- database folder: "+dim.get<VarSystemLibStorage>("StorageInterface")->getDatabseFolder() + "\n")+
            string("+-- Services\n")+
            string("    +-- VSTP port: "+dim.get<VSTP>()->getListeningInfo() + "\n") + 
            string("    +-- HTTP port: "+dim.get<HTTP::HttpAPI>()->getListeningInfo() + "\n") + 
            string("    +-- Server discovery port: "+dim.get<ServerDiscovery>()->getRunningPortInfo() + "\n")
        );
    /* #endregion */

    //prevent program close (equivalent to while (true))
    exitMutex.lock();
    exitMutex.lock();

    //log additional informabation about signals
    if (exitSignal == 4)
        logger->critical("", "Illegal instruction signal received");
    else if (exitSignal == 11)
        logger->critical("", "Segmentation fault signal received");
    else if (exitSignal == 16)
        logger->critical("", "Stack fault signal received");

    /* #region finalization of the program */
        logger->info("", "VSS is exiting. Please wait...");
        //delete controller;
        //delete vstp;
        
        delete dim.get<VSTP>();

        delete dim.get<ServerDiscovery>();

        delete dim.get<TheController>();

        delete dim.get<Config>();

        delete dim.get<StorageInterface>();

        delete dim.get<ThreadPool>();

        delete dim.get<ILogger>();

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
    string directory = appPath.substr(0,found);

    return directory;
}


bool isRunningInPortableMode()
{
    //use the conf file to check if app is runing in a portable mode
    Shared::SysLink sl;
    return sl.fileExists(getApplicationDirectory() + "/confs.conf");
}

std::string findConfigurationFile()
{
    //determine the configuration file
    
    string confFile = "/etc/vss/confs.conf";

    if (isRunningInPortableMode())
        confFile = getApplicationDirectory() + "/confs.conf";

    return confFile;
}

std::string determinteLogFile()
{
    string logFile = "/var/log/vss.log";

    
    if (isRunningInPortableMode())
        logFile = getApplicationDirectory() + "/vss.log";

    return logFile;
}

void handleSignals()
{
    signal(SIGINT, signalHandler);  
    signal(SIGQUIT, signalHandler);  
    signal(SIGABRT, signalHandler);  
    signal(SIGSEGV, signalHandler);  
    signal(SIGTERM, signalHandler);  
    signal(SIGKILL, signalHandler);
}