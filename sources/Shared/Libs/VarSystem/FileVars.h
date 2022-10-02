#ifndef FILEVARS_H
#define FILEVARS_H

#include <iostream>
#include <stdio.h>
#include <string>
#include <strings.h>
#include <unistd.h>
#include <fstream>
#include "SysLink.h"
#include <map>



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
			string directory;
            int ramCacheMaxItems = 10000;
            map<string, Var*> ramCache;
			SysLink sysLink;
			string getFileName(string varName, bool autoCreateDirectory);
            bool useCache = true;
            pthread_t __Thread_syncToFsPointer;
			bool debugMode = false;

			bool running = true;

			bool containsKey(string key);
		public:
			FileVars(string dirBase, bool isToUseCache);
			~FileVars();
			void set(string varName, string value);
			void append(string varName, string value);
			void setDouble(string varName, double value);
			void setLong(string varName, long value);
			void setBool(string varName, bool value);
			void del(string varName);
            void __Thread_syncToFs();

			Var get(string varName, string defaulValue);
			Var getPart(string varName, size_t start, size_t count);
			size_t getVarSize(string varName);

			vector<string> getChilds(string parentName);
	};

    void *ThreadFileVars_syncToFs(void *thisPointer);
}
#endif