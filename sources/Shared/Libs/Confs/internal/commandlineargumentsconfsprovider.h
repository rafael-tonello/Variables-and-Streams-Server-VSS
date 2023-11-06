#ifndef __COMMANDLINEARGUMENTSCONFSPROVIDER__H__ 
#define __COMMANDLINEARGUMENTSCONFSPROVIDER__H__ 

#include "IConfProvider.h"
#include "argparser.h"
 
class CommandLineArgumentsConfsProvider: public IConfProvider { 
private:
    vector<string> varNames;
public: 
    CommandLineArgumentsConfsProvider(); 
    ~CommandLineArgumentsConfsProvider();

    ArgParser ap;

public:
    CommandLineArgumentsConfsProvider(int argc, char **argv);

    /* IConfProvider interface */
    bool contains(string name);
    DynamicVar get(string name);
    void listen(function<void(string, DynamicVar)> f);
    string getTypeIdName();

}; 
 
#endif 
