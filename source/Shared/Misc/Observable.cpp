#include "Observable.h"
void Observable::observe(function<void(string message, void* data)> observer, string messagesPrefix)
{
    tuple<string, function<void(string message, void* data)>> t (messagesPrefix, observer);
    observers.push_back(t);
}

void Observable::notify(string message, void* data)
{
    for (auto &c :observers)
    {
        if (std::get<0>(c) == "" || message.find(std::get<0>(c)) == 0)
        {
            std::get<1>(c)(message, data);
        }
    }
}