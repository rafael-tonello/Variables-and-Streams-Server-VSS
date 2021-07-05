#ifndef _DEPENDENCY_INJECTION_MANAGER_TESTS_H
#define _DEPENDENCY_INJECTION_MANAGER_TESTS_H

#include <tester.h>
#include "../../../../source/Shared/DependencyInjectionManager/DependencyInjectionManager.h"
class DependencyInjectionManagerTester: public Tester{
private:

public:
    vector<string> getContexts();
    void run(string context);

};

#endif