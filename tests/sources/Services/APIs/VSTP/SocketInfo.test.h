#ifndef _ze2p39ryn8_H
#define _ze2p39ryn8_H

#include <string>
#include <regex>
#include <tester.h>
#include <unistd.h>
#include "../../../../../sources/Services/APIs/VSTP/SocketInfo.h"
#include "../../../../../sources/Shared/Libs/ThreadPool/ThreadPool.h"

class SocketInfoTester: public Tester{
private:
    API::SocketInfo socketInfo;

public:
    SocketInfoTester(){};
    vector<string> getContexts();
    void run(string context);

};

#endif