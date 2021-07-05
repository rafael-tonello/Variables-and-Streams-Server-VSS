#ifndef _CONTROLLER_TEST_H_
#define _CONTROLLER_TEST_H_

#include <tester.h>
#include "../../../source/Controller/Controller.h"
#include <map>

using namespace std;
using namespace Controller;
class ControllerTester: public Tester{
private:
    TheController ctrl;

    void testSetVar();
    void testGetVar();
public:
    /*Tester 'interface'*/
    vector<string> getContexts();
    void run(string context);
};

#endif