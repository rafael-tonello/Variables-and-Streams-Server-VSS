#ifndef __LOGFILEWRITER__H__ 
#define __LOGFILEWRITER__H__ 

#include <fstream>
#include "../logger.h"
#include <ctime>
 
using namespace std;
class LoggerFileWriter: public ILogWriter
{		
private:
	bool _writeDate;
    bool _writeTime;
	ofstream file;
	string getDate();
	string getTime();
	string generateDateTimeString();
public:
	LoggerFileWriter(string fname, bool writeDate = true, bool writeTime = true);
	~LoggerFileWriter();
	void write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel);
};
 
#endif 
