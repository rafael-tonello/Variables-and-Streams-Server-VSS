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
	LoggerFileWriter(string fname)
	{
		file = ofstream(fname);
		if (!file.is_open())
			throw runtime_error("Can't open log file to write");
		
	}
	void write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel){
		file << (name != "" ? "["+ name + "]" : "") << msg << endl;
	}
};
 
#endif 
