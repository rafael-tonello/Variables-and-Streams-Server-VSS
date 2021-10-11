#ifndef _SIMPLE_CONF_FILE_PROVIDER_TESTS_H
#define _SIMPLE_CONF_FILE_PROVIDER_TESTS_H

#include <tester.h>
#include "../../../../../source/Shared/Confs/internal/SimpleConfFileProvider.h"
#include <fstream>
using namespace Shared;
class SimpleConfFileProviderTester: public Tester{
private:

public:
    vector<string> getContexts();
    void run(string context);

};

#endif