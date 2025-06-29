#include  "commandlineargumentsconfsprovider.h" 
 
CommandLineArgumentsConfsProvider::CommandLineArgumentsConfsProvider(int argc, char **argv) 
{ 
    ap = ArgParser(argc, argv);
     
} 
 
CommandLineArgumentsConfsProvider::~CommandLineArgumentsConfsProvider() 
{ 
     
} 

bool CommandLineArgumentsConfsProvider::contains(string name)
{
    return ap.contains(name);
}

DynamicVar CommandLineArgumentsConfsProvider::get(string name)
{
    return ap.get(name);
}

void CommandLineArgumentsConfsProvider::listen(function<void(string, DynamicVar)> f)
{

}

string CommandLineArgumentsConfsProvider::getTypeIdName()
{
    return typeid(CommandLineArgumentsConfsProvider).name();
}

