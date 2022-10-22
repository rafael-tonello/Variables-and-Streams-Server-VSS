#ifndef __MESSAGEBUS__H__ 
#define __MESSAGEBUS__H__ 

#include<vector>
#include<map>
#include<functional>
#include<mutex>
#include <ThreadPool.h>

#include <JSON.h>

using namespace std;
 
template <typename T>
using OnMessageF = function<void(string message, T args, T &result)>;


template <typename T>
class MessageBus
{
    private:
        uint currId = 0;
        map<uint, tuple<string, OnMessageF<T>>> observers;
        
        function<bool(T item)> isEmpty;
        ThreadPool *scheduler;
    public:
        MessageBus(ThreadPool *scheduler, function<bool(T item)> isEmpty): isEmpty(isEmpty), scheduler(scheduler){}
        
        vector<T> post(string message, T args)
        {
            vector<T> result;
            mutex resultLock;
            
            auto tmpObservers = getObservers(message);
            vector<future<void>> pending;
            for (auto &c: tmpObservers)
            {
                pending.push_back(scheduler->enqueue, [&](OnMessageF<T> observer)
                {
                    T tmpResult;
                    observer(message, args, tmpResult);
                    if (!isEmpty(tmpResult))
                    {
                        resultLock.lock();
                        result.push_back(tmpResult);
                        resultLock.unlock();
                    }
                }, c);
            }
            
            for (auto &currPending: pending)
            {
                currPending.wait();
            }
            
            return result;
        }
        
        uint listen(string message, OnMessageF<T> f)
        {
            auto observerId = currId++;
            observers[observerId] = {message, f};
        }

        void stopListen(int observerId)
        {
            if (observers.count(observerId))
                observers.erase(observerId);
        }

        vector<OnMessageF<T>> getObservers(string message)
        {
            vector<OnMessageF<T>> result;
            for(auto &c: observers)
            {
                string currObserverMessagePattern = std::get<0>(c.second);
                
                if (message.find(currObserverMessagePattern) != string::npos)
                    result.push_back(std::get<1>(c.second));
            }
            
            return result;
        }

        future<T> waitNext(string message)
        {
            promise<T> *prom = new promise<T>;
            int *id = new int();
            *id =  this->listen(message, [&](string message, T data){
                prom->set_value(data);
                this->stopListen(*id);
                delete id;
                delete prom;
            });
            return prom->get_future();
        }
        
        static MessageBus<JsonMaker::JSON> createJsonMessageBus(){ 
            return MessageBus<JsonMaker::JSON>([](JsonMaker::JSON item){
                return item.getChildsNames("").size() == 0;
            });
        }
        
        static MessageBus<string> createStringMessageBus(){ 
            return MessageBus<string>([](string item){
                return item == "";
            });
        } 
};
 
#endif 
