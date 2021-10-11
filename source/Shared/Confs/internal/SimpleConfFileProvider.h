/*
    this configuration provider reads a simple configuration file, that is very common in the Linux ambience.
    this kind of file are key-value pair, where key and value are separed by double dot (:) or a equals (=) character

*/
#include <vector>
#include <tuple>
#include <string>
#include <fstream>
#include "IConfigurationProvider.h"

using namespace std;
namespace Shared{
    class SimpleConfFileProvider: public IConfigurationProvider
    {
    private:
    #ifdef __TESTING__
        public:
    #endif
        const string separatorChars = "=:";
        string filename;
        string ltrim(string str);
        string rtrim(string str);
        string identifyKeyValueSeparator(string str);
    public:
        SimpleConfFileProvider(string filename);
    
    //{IConfigurationProvider interface}
    public:
        vector<tuple<string, string>> readAllConfigurations();
    };
}