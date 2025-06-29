#ifndef _DEPENDENCY_INJECTION_MANAGER_TESTS_H
#define _DEPENDENCY_INJECTION_MANAGER_TESTS_H

#include <tester.h>
#include <dependencyInjectionManager.h>
class DependencyInjectionManagerTester: public Tester{
private:
    void testsUsingTypes();
    void testsUsingNames();

public:
    vector<string> getContexts();
    void run(string context);

};

#endif