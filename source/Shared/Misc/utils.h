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

using namespace std;
    using named_lock_f = function<void()>;
    
    class Utils{
    private:
    #ifdef D__TESTING__
    private: 
    #endif
        static mutex names_locks_mutexes_mutex;
        static map<string, mutex*> names_locks_mutexes;
        static int idCount;
        static bool srandOk;

        static void runSRand();
        static void initializeProxyList();


        static vector <string> validProxies;
        static char proxyListInitializedState;
    public:
        static void named_lock(string session_name, named_lock_f f);
        static int64_t getCurrentTimeMilliseconds();
        static int64_t getCurrentTimeSeconds();

        static string StringToHex(string& input);
        static string ssystem (string, bool removeTheTheLastLF = true);
        static future<string> asystem(string, bool removeTheTheLastLF = true);
        static future<string> httpGet(string url, map<string, string> headers = {});
        static future<string> httpPost(string url, string body, string contentType = "application/json", map<string, string> headers = {});
        static void process_mem_usage(double& vm_usage, double& resident_set);
        static double process_vm_usage();
        static double process_resident_usage();
        static string createUniqueId();

        static string readTextFileContent(string fileName);
        static void writeTextFileContent(string fileName, string content);
        static void appendTextFileContent(string fileName, string content);

        static string findAndReplaceAll(std::string data, std::string toSearch, std::string replaceStr);

        static string downloadWithRandomProxy(string url, string destFileName, int maxTries = 5);


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
