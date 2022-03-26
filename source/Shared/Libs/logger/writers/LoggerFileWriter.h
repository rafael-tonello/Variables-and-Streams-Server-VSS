#ifndef __LOGFILEWRITER__H__ 
#define __LOGFILEWRITER__H__ 

#include <fstream>
#include "../logger.h"
 
using namespace std;
class LoggerFileWriter: public ILogWriter
{		
private:
	ofstream file;
public:
	LoggerFileWriter(string fname);
	~LoggerFileWriter();
	void write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel);
};
 
#endif 
