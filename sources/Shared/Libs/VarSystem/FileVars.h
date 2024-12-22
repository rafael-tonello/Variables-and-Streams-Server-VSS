#ifndef FILEVARS_H
#define FILEVARS_H

#include <iostream>
#include <stdio.h>
#include <string>
#include <strings.h>
#include <unistd.h>
#include <fstream>
#include "FVSysLink.h"
#include <map>
#include <thread>
#include <mutex>



using namespace std;

namespace Shared
{
	enum VarStateOnFs {AlreadySync, DataToBeSet, DataToBeAppend};
	class Var
	{
		private:
			string _data;
		public:
            VarStateOnFs varStateOnFs = AlreadySync;

			Var(string data);
			Var(char* buffer, size_t size);
            void set(string newData);
			void append(string newData);
			string AsString();
			double AsDouble();
			long AsLong();
			bool AsBool();
			size_t size();
			
	};

	class FileVars
	{
		private:
			size_t maxCacheSize = 10 * 1024 * 1024; //10 MB buffer
			size_t currentCacheSize = 0;
			string directory;
            int ramCacheMaxItems = 10000;
            map<string, Var*> ramCache;
			SysLink sysLink;
			string getFileName(string varName, bool autoCreateDirectory);
            bool useCache = true;
            pthread_t __Thread_syncToFsPointer;
			bool debugMode = false;

			bool running = true;

			mutex ramCacheLocker;

			bool containsKey(string key);

			void deleteDirectoryTree(string directory);
		public:
			FileVars(string dirBase, bool isToUseCache, size_t inMemoryCacheSize = 10 * 1024 * 1024);
			~FileVars();
			void set(string varName, string value);
			void append(string varName, string value);
			void setDouble(string varName, double value);
			void setLong(string varName, long value);
			void setBool(string varName, bool value);
			void del(string varName);
            void __Thread_syncToFs();

			//using initializeFileAndCahceWithDefaultValue as true, the get vars speed will be increaseed, because the system will automatically
			//initializing the cache with the default value. The nexe file time the variabel will be acccessed, the file will no be read.
			//if the falue is false, every time (if variable do not exists) the system will look for its value in the filesytem.
			Var get(string varName, string defaulValue, bool initializeFileAndCahceWithDefaultValue = false);
			Var getPart(string varName, size_t start, size_t count);
			size_t getVarSize(string varName);

			vector<string> getChilds(string parentName);
	};
}
#endif