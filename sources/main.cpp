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
#include <VarSystem/FVSysLink.h>
#include <logger.h>
#include <LoggerConsoleWriter.h>
#include <LoggerFileWriter.h>
#include <ServerDiscovery.h>
#include <Confs/internal/SimpleConfFileProvider.h>
#include <Confs/internal/soenvironmentconfprovider.h>
#include <Confs/internal/commandlineargumentsconfsprovider.h>
#include <Storage/RamCacheDB/ramcachedb.h>

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

Confs* initConfigurations(int argc, char** argv);

//semantic versioning
//ATTENTION: to change version, use apply_version_number.sh script in the project root folder (read the comments at the beginning of the script before use it)
string INFO_VERSION = "v1.11.1+Haumea";

#define KIB *1024
#define MIB *1024*1024


int main(int argc, char** argv){
    handleSignals();
    srand((unsigned)time(0)); 
    
    DependencyInjectionManager dim;

    dim.addSingleton<string>(&INFO_VERSION, {"version", "systemVersion", "infoVersion", "INFO_VERSION", "SYSTEM_VERSION"});

    dim.addSingleton<Confs>(initConfigurations(argc, argv));
    
    dim.addSingleton<ILogger>(new Logger({
        new LoggerConsoleWriter(LOGGER_LOGLEVEL_INFO2),
        new LoggerFileWriter(determinteLogFile(), LOGGER_LOGLEVEL_INFO2, true, dim.get<Confs>()->getA("maxLogFileSize", 50 MIB).getInt())
    }, false));
    
    dim.addSingleton<ThreadPool>(new ThreadPool(20, 0, "VSSTHPOOL_"));
    
    //threadpool2 can be used to prevent deadlocks with the main ThreadPool1. You also can use threadpool1 withtout threadlimit to prevent deadlocks
    dim.addSingleton<ThreadPool>(new ThreadPool(10, 0, "VSSTHPOOL2_"), { "ThreadPool2" }); 
    
    dim.addSingleton<MessageBus<JsonMaker::JSON>>(new MessageBus<JsonMaker::JSON>(dim.get<ThreadPool>(), [](JsonMaker::JSON &item){return item.getChildsNames("").size() == 0;}));

    //dim.addSingleton<StorageInterface>(new VarSystemLibStorage(&dim));
    dim.addSingleton<StorageInterface>(new RamCacheDB(&dim));
    
    /*two pointers to controller (to allow systems to find it by all it types):
     the controller can be find by use of get<TheController> and get<ApiMediatorInterface>*/
    dim.addSingleton<TheController>(new TheController(&dim, INFO_VERSION), {typeid(TheController).name(), typeid(ApiMediatorInterface).name()});
    dim.addSingleton<VSTP>(new VSTP(dim.get<Confs>()->getA("vstpApiPort"), dim));
    dim.addSingleton<API::HTTP::HttpAPI>(new API::HTTP::HttpAPI(dim.get<Confs>()->getA("httpApiPort"), dim.get<Confs>()->getA("httpApiHttpsPort"), &dim));
    dim.addSingleton<ServerDiscovery>(new ServerDiscovery(dim, INFO_VERSION));


    auto logger = dim.get<ILogger>();

    /* #region Initial information messages */
    string initialInfo = string("The VSS has been started\n") +
        string("+-- Version: "+ INFO_VERSION + "\n")+
        string("+-- Portable mode: ")+ string(isRunningInPortableMode() ? "Yes":"No")+ "\n" + 
        string("|   +-- conf file: "+findConfigurationFile()+"\n")+
        string("|   +-- log file: "+determinteLogFile()+"\n")+
        string("|   +-- database folder: "+dim.get<VarSystemLibStorage>("StorageInterface")->getDatabseFolder() + "\n")+
        string("+-- Services\n")+
        string("|   +-- VSTP port: "+dim.get<VSTP>()->getListeningInfo() + "\n") + 
        string("|   +-- HTTP port: "+dim.get<HTTP::HttpAPI>()->getListeningInfo() + "\n") + 
        string("|   +-- Server discovery port: "+dim.get<ServerDiscovery>()->getRunningPortInfo() + "\n") +
        string("+-- All configurations\n")
    ;

    for (auto &c: dim.get<Confs>()->getAllConfigurationsA())
    {
        auto key = std::get<0>(c);
        auto value = std::get<1>(c).getString();
        initialInfo += "    +-- " + key + ": " + value + "\n";
    }

    logger->info("", initialInfo);
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

        delete dim.get<Confs>();

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


Confs* initConfigurations(int argc, char **argv)
{
    Confs *conf = new Confs();

    conf->addProvider(new CommandLineArgumentsConfsProvider(argc, argv));
    conf->addProvider(new SoEnvironmentConfProvider());
    conf->addProvider(new Shared::SimpleConfFileProvider(findConfigurationFile()));


    conf->createPlaceHolders()
        .add("%PROJECT_DIR%", getApplicationDirectory())
        .add("%APP_DIR%", getApplicationDirectory())
        .add("%FILE_SYSTEM_CONTEXT%", isRunningInPortableMode() ? getApplicationDirectory() : "")
        .add("%SUGGESTED_DATA_DIRECTORY%", isRunningInPortableMode() ? getApplicationDirectory() + "/data" : "/var/vss/data")
    ;

    //max log file size (logger library will compress the file if it is bigger than this value)
    conf->createAlias("maxLogFileSize").addForAnyProvider({"maxLogFileSize", "--maxLogFileSize", "VSS_MAX_LOG_FILE_SIZE"}).setDefaultValue(50 MIB);
    
    //max time to consider a client complettly disconnected (not just a network problem)
    conf->createAlias("maxTimeWaitingClient_seconds").addForAnyProvider({"maxTimeWaitingClient_seconds", "--maxTimeWaitingForClients", "VSS_MAX_TIME_WAITING_CLIENTS"}).setDefaultValue(12*60*60);

    //where database files should be stored
    conf->createAlias("DbDirectory").addForAnyProvider({"dbDirectory", "--dbDirectory", "VSS_DB_DIRECTORY"}).setDefaultValue("%SUGGESTED_DATA_DIRECTORY%/database");

    //HTTP API
        //directory to store http data (temp files, cookies, ...)
        conf->createAlias("httpDataDir").addForAnyProvider({"httpDataDirectory", "--httpDataFolder", "--httpDataDirectory", "--httpDataDir", "VSS_HTTP_DATA_DIRECTORY"}).setDefaultValue("%SUGGESTED_DATA_DIRECTORY%/http_data");
        //HTTP port (note that it different from the HTTPS port)
        conf->createAlias("httpApiPort").addForAnyProvider({"httpApiPort", "--httpApiPort", "VSS_HTTP_API_PORT"}).setDefaultValue(5024);

        //HTTPS port
        conf->createAlias("httpApiHttpsPort").addForAnyProvider({"httpApiHttpsPort", "--httpApiHttpsPort", "VSS_HTTP_API_HTTPS_PORT"}).setDefaultValue(5025);
        //HTTPs certificate and key files
        conf->createAlias("httpApiCertFile").addForAnyProvider({"httpApiCertFile", "--httpApiCertFile", "VSS_HTTP_API_CERT_FILE"}).setDefaultValue("%APP_DIR%/ssl/cert/vssCert.pem");
        conf->createAlias("httpApiKeyFile").addForAnyProvider({"httpApiKeyFile", "--httpApiKeyFile", "VSS_HTTP_API_KEY_FILE"}).setDefaultValue("%APP_DIR%/ssl/cert/vssKey.pem");


        conf->createAlias("httpApiReturnFullPaths").addForAnyProvider({"httpApiReturnFullPaths", "--httpApiReturnFullPaths", "VSS_HTTP_API_RETURN_FULL_PATHS"}).setDefaultValue(false);

    //VSTP API
        conf->createAlias("vstpApiPort").addForAnyProvider({"vstpApiPort", "--vstpApiPort", "VSS_VSTP_API_PORT"}).setDefaultValue(5032);


    //RamCacheDB
        //time interval to dump the database to disk (for Storage Driver RamCacheDB)
        conf->createAlias("RamCacheDbDumpIntervalMs").addForAnyProvider({"RamCacheDbDumpIntervalMs", "--RamCacheDbDumpIntervalMs", "VSS_RAMCACHEDB_DUMP_INTERVAL_MS"}).setDefaultValue(60*1000);

    return conf;
}