#ifndef _SIMPLE_CONF_FILE_PROVIDER_TESTS_H
#define _SIMPLE_CONF_FILE_PROVIDER_TESTS_H

#include <tester.h>
#include <Confs/internal/SimpleConfFileProvider.h>
#include <fstream>
using namespace Shared;
class SimpleConfFileProviderTester: public Tester{
private:
    string defaultFile = "/tmp/SimpleConfFileProvider_fileForTests.tmp";
    void writeFile(string data);
    void writeFile(string fName, string data);

public:
    vector<string> getContexts();
    void run(string context);

};

#endif