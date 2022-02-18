#ifndef _CONFS_FILE_PROVIDER_TESTS_H
#define _CONFS_FILE_PROVIDER_TESTS_H

#include <vector>
#include <tester.h>
#include <Confs.h>

using namespace Shared;
using namespace std;
class ConfsTester: public Tester{
private:

public:
    vector<string> getContexts();
    void run(string context);
};

#endif
