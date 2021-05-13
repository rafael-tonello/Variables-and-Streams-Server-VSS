#ifndef _ze2p39ryn8_H
#define _ze2p39ryn8_H

#include <string>
#include <regex>
#include <tester.h>
#include <unistd.h>
#include "../../../../../source/Services/APIs/PHOMAU/SocketInfo.h"
#include "../../../../../source/Shared/ThreadPool/ThreadPool.h"

class SocketInfoTester: public Tester{
private:
    API::SocketInfo socketInfo;

public:
    SocketInfoTester(){};
    vector<string> getContexts();
    void run(string context);

};

#endif