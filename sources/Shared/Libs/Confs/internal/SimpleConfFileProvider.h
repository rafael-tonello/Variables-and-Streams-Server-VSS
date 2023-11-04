/*
    this configuration provider reads a simple configuration file, that is very common in the Linux ambience.
    this kind of file are key-value pair, where key and value are separed by double dot (:) or a equals (=) character

*/
#ifndef __SIMPLE_CONF_PROVIDER_H__
#define __SIMPLE_CONF_PROVIDER_H__

#include <vector>
#include <tuple>
#include <string>
#include <fstream>
#include "IConfProvider.h"
#include <mutex>
#include <atomic>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <future>
#include <DynamicVar.h>
#include <map>

using namespace std;
namespace Shared{
    class SimpleConfFileProvider: public IConfProvider
    {
    private:
    #ifdef __TESTING__
        public:
    #endif
        const string separatorChars = "=:";
        string filename;

        map<string, DynamicVar> currValues;

        function<void(string, DynamicVar)> _onData;



        mutex threadExitingMutex;
        atomic<bool> runing;
        //a long poll file check
        void fileCheckPoll();

        long int getFileChangeTime(string fname);
        void readAndNotify();
        string ltrim(string str);
        string rtrim(string str);
        string identifyKeyValueSeparator(string str);
        vector<tuple<string, string>> readAllConfigurations();

    public:
        SimpleConfFileProvider(string filename);
        ~SimpleConfFileProvider();
    
    //{IConfProvider interface}
    public:
        bool contains(string name) override;
        DynamicVar get(string name) override;
        void listen(function<void(string, DynamicVar)> f) override;
        string getTypeIdName() override 
        { 
            string ret = typeid(SimpleConfFileProvider).name();
            return ret;
        };
    };
}

#endif