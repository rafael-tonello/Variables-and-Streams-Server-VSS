#ifndef __TIMERSFORDEBUG__
#define __TIMERSFORDEBUG__

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <utils.h>

using namespace std;
class __TimersForDebugInfo
{
public:
    string name = "";
    uint64_t totalTime_us = 0;
    uint64_t currStart = 0;
    uint steps = 0;

    double getAverageStepTime_ms(){
        double ret = getAverageStepTime_us() / 1000.f;
        return ret;
    }

    double getAverageStepTime_us(){
        double ret = ((double)totalTime_us)/((double)steps);
        return ret;
    }
};

class TimersForDebug{
private:
    static map<string, __TimersForDebugInfo> timers;
public:
    static void begin(string name);
    static void reset(string name){begin(name);}
    static void end(string name);
    static void step(string name){ end(name); }

    static void clear();

    static __TimersForDebugInfo get(string name);
    static vector<__TimersForDebugInfo> getAll();
};

#endif