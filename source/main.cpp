#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <signal.h>


#include "Controller/Controller.h"
#include "Services/APIs/PHOMAU/PHOMAU.h"

using namespace std;
using namespace Controller;
using namespace API;

mutex exitMutex;

void signalHandler( int signum ) {
   cout << "Interrupt signal (" << signum << ") received.\n";
   exitMutex.unlock();
}

int main(){
    
    signal(SIGINT, signalHandler);  
    signal(SIGQUIT, signalHandler);  
    signal(SIGABRT, signalHandler);  
    signal(SIGSEGV, signalHandler);  
    signal(SIGTERM, signalHandler);  
    signal(SIGKILL, signalHandler);  


    //the application main controller
    TheController* controller = new TheController();

    //#region the other services and dependency injection
        PHOMAU* phomau = new PHOMAU((int)5021, controller);
    //#endregion
    
    /* #region Initial information messages */
        cout << "the system is running" << endl;
        cout << "\tPHOMAU API port: 5021" << endl;
    /* #endregio n*/

    //prevent program close (equivalent to while (true))
    exitMutex.lock();
    exitMutex.lock();

    /* #region finalization of the program */
        cout << "The program is exiting. Please wait..." <<endl;
        delete controller;
        delete phomau;
    /* #endregion */

    exit(0);  
    return 0;
}