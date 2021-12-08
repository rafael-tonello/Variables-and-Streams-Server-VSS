#ifndef __LOGGER__H__ 
#define __LOGGER__H__ 

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
using namespace std;


	
class ILogger{
public:
	virtual void log(string msg, int level, string name = "") = 0;
	virtual void debug(string msg, string name="") = 0;
	virtual void info(string msg, string name="") = 0;
	virtual void warning(string msg, string name="") = 0;
	virtual void error(string msg, string name="") = 0;
	virtual void critical(string msg, string name="", bool raiseException = true) = 0;
};

class Logger;

class ILogWriter{
public:
	virtual void write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel) = 0;
};


class Logger: public ILogger{
private:
	vector<ILogWriter*> writers;
public:
	int logLevel;

	static int LOGGER_DEBUG_LEVEL;
	static int LOGGER_INFO_LEVEL;
	static int LOGGER_WARNING_LEVEL;
	static int LOGGER_ERROR_LEVEL;
	static int LOGGER_CRITICAL_LEVEL;
	
	Logger(vector<ILogWriter*> writers, int logLevel = LOGGER_INFO_LEVEL);
	void log(string msg, int level, string name="");	
	void debug(string msg, string name="");	
	void info(string msg, string name="");	
	void warning(string msg, string name="");	
	void error(string msg, string name="");	
	void critical(string msg, string name="", bool raiseException = true);
};

#endif 
