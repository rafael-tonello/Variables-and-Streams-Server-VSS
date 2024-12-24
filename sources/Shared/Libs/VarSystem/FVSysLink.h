#ifndef _VARSYSTEM_SYSLINK_H
#define _VARSYSTEM_SYSLINK_H

#define plataform_autoreplaced_by_command_line

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#ifdef WIN_32
	#include <windows.h>
#endif
#ifdef Linux
	#include <unistd.h>
#endif



using namespace std;

namespace Shared {
	class SysLink
	{
		private:
			vector<string> split(string* text, char sep);
		public:
			//file system
			bool fileExists(string filename);
			bool deleteFile(string filename);
			bool writeFile(string filename, string data);
			bool appendFile(string filename, string data);
			string readFile(string filename);
			void readFile(string filename, char* buffer, size_t start, size_t count);
			size_t getFileSize(string filename);
			bool waitAndLockFile(string filename, int maxTimeout = 1000);
			bool unlockFile(string filename);
			bool directoryExists(string directoryName);
			bool createDirectory(string directoryName);
			vector<string> getObjectsFromDirectory(string directoryName, string lsFilter, string grepArguments);
			vector<string> getFilesFromDirectory(string directoryName, string searchPatern);
			vector<string> getDirectoriesFromDirectory(string directoryName, string searchPatern);
			bool deleteDirectory(string directoryName);
			string getFileName(string path);
			string getDirectoryName(string path);

			//misc
			void sleep_ms(unsigned int ms);
	};
}
#endif