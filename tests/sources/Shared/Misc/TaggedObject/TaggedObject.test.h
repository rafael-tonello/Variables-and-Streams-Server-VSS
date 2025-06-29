#ifndef __TAGGEDOBJECTTEST__H__ 
#define __TAGGEDOBJECTTEST__H__ 

#include <string>
#include <tester.h>
#include <TaggedObject.h>

class TaggedObjectTesterTempClass: public TaggedObject{

};

class TaggedObjectTester: public Tester { 
public: 
    TaggedObjectTester(); 
    ~TaggedObjectTester(); 

    vector<string> getContexts();
    void run(string context);
}; 
 
#endif 
