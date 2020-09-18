#ifndef _OBSERVABLE_H_
#define _OBSERVABLE_H_

#include <string>
#include <tuple>
#include <vector>
#include <functional>

using namespace std;
class Observable{
private:
    vector<tuple<string, function<void(string, void*)>>> observers;
public:
    void observe(function<void(string message, void* data)> observer, string messagesPrfix="");
    void notify(string message, void* data = NULL);
};

#endif