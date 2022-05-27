#ifndef __LOGCONSOLEWRITER__H__ 
#define __LOGCONSOLEWRITER__H__ 

#include <iostream>
#include <string>
#include "../logger.h"

using namespace std;
 
class LoggerConsoleWriter: public ILogWriter
{
private:
    bool colors = true;
public:
    LoggerConsoleWriter(bool useColors);
	void write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel);
};
 
#endif 
