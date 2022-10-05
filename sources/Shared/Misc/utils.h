#ifndef __UTILS__H__
#define __UTILS__H__

#include <string>
#include <fstream>
#include <future>
#include <map>
#include <unistd.h>
#include <exception>
#include <ThreadPool.h>
#include <sys/wait.h>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std;
    using named_lock_f = function<void()>;
    
    class Utils{
    private:
    #ifdef D__TESTING__
    private: 
    #endif
        static mutex names_locks_mutexes_mutex;
        static map<string, timed_mutex*> names_locks_mutexes;
        static int idCount;
        static bool srandOk;

        static void runSRand();
        static void initializeProxyList();


        static vector <string> validProxies;
        static char proxyListInitializedState;
    public:
        static void named_lock(string session_name, named_lock_f f, int timeout_ms = -1);
        static void named_lock_forceunlock(string session_name);
        static int64_t getCurrentTimeMicroseconds();
        static int64_t getCurrentTimeMilliseconds();
        static int64_t getCurrentTimeSeconds();

        static string StringToHex(string& input, size_t size);
        static string StringToHex(string& input);
        static string charVecToHex(char* data, size_t size);
        static string charVecToHex(const char* data, size_t size);
        static string ssystem (string, bool removeTheLastLF = true);
        static future<string> asystem(string, bool removeTheLastLF = true);
        static future<string> httpGet(string url, map<string, string> headers = {});
        static future<string> httpPost(string url, string body, string contentType = "application/json", map<string, string> headers = {});
        static void process_mem_usage(double& vm_usage, double& resident_set);
        static double process_vm_usage();
        static double process_resident_usage();
        static string createUniqueId();

        static string readTextFileContent(string fileName);
        static void writeTextFileContent(string fileName, string content);
        static void appendTextFileContent(string fileName, string content);

        // static string findAndReplaceAll(std::string data, std::string toSearch, std::string replaceStr);

        static string downloadWithRandomProxy(string url, string destFileName, int maxTries = 5);

        static string stringReplace(string source, string replace, string by);
        static string stringReplace(string source, vector<tuple<string, string>> replaceAndByTuples);
        static string sr(string source, string replace, string by){return stringReplace(source, replace, by);};
        static string sr(string source, vector<tuple<string, string>> replaceAndByTuples){return stringReplace(source, replaceAndByTuples);};
        
        static bool isNumber(string source);

        static map<void*, string> getANameDB;
        enum NameType{ALGORITHM_GENERATED, REAL_NAME_COMBINATION};
        static string getAName(int number, NameType typeOfName = ALGORITHM_GENERATED, int AlgoGenMaxSyllables = 3);
        static string getAName(void* p, NameType typeOfName = ALGORITHM_GENERATED, int AlgoGenMaxSyllables = 3);

        static string getNestedExceptionText(exception &e, string prefix ="", int level = 0);


        template<typename T>
        static future<void> parallel_foreach(vector<T> items, function<void(T)> f, ThreadPool *tasker)
        {
            vector<future<void>> pendingTasks = {};
            for (auto &c: items)
            {
                pendingTasks.push_back(tasker->enqueue([&](T &item){
                    f(item);
                }, c));
            };

            /*return tasker->enqueue([&](auto pendingTasks2){
                for (auto &c: pendingTasks2)
                    c.wait();
            }, pendingTasks);*/

            return tasker->enqueue([&](){
                for (auto &c: pendingTasks)
                    c.wait();
            });
        }

        static future<void> parallel_for(int from, int to, function<void(int)> f, ThreadPool * tasker)
        {
            vector<future<void>> pendingTasks;
            for (int c = from; c != to; from > to ? c-- : c++)
            {
                if (c != to)
                    pendingTasks.push_back(tasker->enqueue([&](int index){
                        f(index);
                    }, c));
            }

            return tasker->enqueue([&](){
                for (auto &c: pendingTasks)
                    c.wait();
            });
        }


    };

#endif
