/*
    this configuration provider reads a simple configuration file, that is very common in the Linux ambience.
    this kind of file are key-value pair, where key and value are separed by double dot (:) or a equals (=) character

*/
#include <vector>
#include <tuple>
#include <string>
#include <fstream>
#include "IConfigurationProvider.h"
#include <mutex>
#include <atomic>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <future>

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
        IConfigurationProvider_onData _onData;



        mutex threadExitingMutex;
        atomic<bool> runing;
        //a long poll file check
        void fileCheckPoll();

        time_t getFileChangeTime(string fname);
        void readAndNotify();
        string ltrim(string str);
        string rtrim(string str);
        string identifyKeyValueSeparator(string str);
        vector<tuple<string, string>> readAllConfigurations();

    public:
        SimpleConfFileProvider(string filename);
        ~SimpleConfFileProvider();
    
    //{IConfigurationProvider interface}
    public:
        void readAndObservate(IConfigurationProvider_onData onData);
    };
}