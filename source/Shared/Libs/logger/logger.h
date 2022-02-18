#ifndef __LOGGER__H__ 
#define __LOGGER__H__ 

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <DynamicVar.h>

using namespace std;
using namespace Shared;

	
class ILogger{
public:
	virtual void log(string name, string msg, int level) = 0;
	virtual void debug(string name, string msg) = 0;
	virtual void info(string name, string msg) = 0;
	virtual void warning(string name, string msg) = 0;
	virtual void error(string name, string msg) = 0;
	virtual void critical(string name, string msg, bool raiseException = true) = 0;

	virtual void log_v(string name, vector<DynamicVar> msgs, int level) = 0;
	virtual void debug_v(string name, vector<DynamicVar> msgs) = 0;
	virtual void info_v(string name, vector<DynamicVar> msgs) = 0;
	virtual void warning_v(string name, vector<DynamicVar> msgs) = 0;
	virtual void error_v(string name, vector<DynamicVar> msgs) = 0;
	virtual void critical_v(string name, vector<DynamicVar> msgs, bool raiseException = true) = 0;

	static string fromList(vector<DynamicVar> items){
		string out;
		for (size_t i = 0; i < items.size(); i++)
			out += items[i].getString() + (i <items.size()-1? " ": "");

		return out;
	}
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
	void log(string name, string msg, int level);
	void debug(string name, string msg);
	void info(string name, string msg);
	void warning(string name, string msg);
	void error(string name, string msg);
	void critical(string name, string msg, bool raiseException = true);

	void log_v(string name, vector<DynamicVar> msgs, int level);
	void debug_v(string name, vector<DynamicVar> msgs);
	void info_v(string name, vector<DynamicVar> msgs);
	void warning_v(string name, vector<DynamicVar> msgs);
	void error_v(string name, vector<DynamicVar> msgs);
	void critical_v(string name, vector<DynamicVar> msgs, bool raiseException = true);
};

#endif 
