#ifndef VSTP_TEST_H
#define VSTP_TEST_H

#include <string>
#include <regex>
#include <tester.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../../../../../sources/Services/APIs/VSTP/VSTP.h"
#include "../../../../../sources/Services/APIs/ApiMediatorInterface.h"
#include <logger.h>
#include <LoggerConsoleWriter.h>

using namespace API;
using namespace TCPServerLib;
class  VstpTester: public Tester{
private:
    
public:
    VstpTester(){
        
    };
    vector<string> getContexts();
    void run(string context);
};

        


#endif