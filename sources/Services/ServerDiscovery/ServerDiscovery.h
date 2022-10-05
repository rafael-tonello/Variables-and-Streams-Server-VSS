#ifndef __SERVERDISCOVERY__H__ 
#define __SERVERDISCOVERY__H__ 

#include <thread>
#include <vector>
#pragma region include for networking
     //#include <sys/types.h>
     #include <fcntl.h>
     //#include <sys/stat.h>
     //#include <errno.h>
     //#include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    // #include <sys/ioctl.h>
    // #include <signal.h>
    // #include <arpa/inet.h>
#pragma endregion

#include <logger.h>
#include <utils.h>
#include <dependencyInjectionManager.h>


using namespace std;
class ServerDiscovery { 
private:
    int port = 5022;
    NLogger *log;
    bool running = true;

    bool SetSocketBlockingEnabled(int fd, bool blocking);
    void run();
    bool IsAServerSearchMessage(string message);
    string getServerInfo();
    vector<string> getServerIps();

    //the ssystem inside Utils.h  is not working weel with '&>>' pipe redirect and must be tested
    string ssystem (string, bool removeTheLastLF = true);
public: 
    ServerDiscovery(DependencyInjectionManager &dim); 
    ~ServerDiscovery(); 
}; 
 
#endif 
