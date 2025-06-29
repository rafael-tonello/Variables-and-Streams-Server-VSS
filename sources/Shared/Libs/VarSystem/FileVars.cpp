#include "FileVars.h"

namespace Shared
{	
	void FileVars::append(string varName, string value)
	{
		string cacheName = this->getFileName(varName, false);
        varName += ".__value__";

		if (this->debugMode)
			cout << "Append var "<<varName << endl << flush;
        if (useCache)
        {
			ramCacheLocker.lock();
            if (this->ramCache.count(cacheName) > 0)
			{
				if (this->ramCache[cacheName]->varStateOnFs == AlreadySync)
					this->ramCache[cacheName]->set(value);	
				else
					this->ramCache[cacheName]->append(value);
			}
            else
                this->ramCache[cacheName] = new Var(value);

			currentCacheSize += value.size();

            this->ramCache[cacheName]->varStateOnFs = DataToBeAppend;
			ramCacheLocker.unlock();
        }
        else
        {
			varName = this->getFileName(varName, false);
    		sysLink.waitAndLockFile(varName, 1000);
    		sysLink.appendFile(varName, value);
    		sysLink.unlockFile(varName);

			if (this->debugMode)
				cout << "Writed file"<<varName << endl << flush;
        }
	}

	FileVars::FileVars(string dirBase, bool isToUseCache, size_t inMemoryCacheSize)
	{
		maxCacheSize = inMemoryCacheSize;

        this->useCache = isToUseCache;
		if (dirBase.back() != '/')
			dirBase += '/';

		this->directory = dirBase;

		thread th([&](){
			__Thread_syncToFs();
		});
        th.detach();
	}

	FileVars::~FileVars()
	{
		running = false;
	}

	//change the value of a variable
	void FileVars::set(string varName, string value)
	{
        varName += ".__value__";
		string cacheName = this->getFileName(varName, false);

		if (this->debugMode)
			cout << "Set var "<<varName << endl << flush;
        if (useCache)
        {
			ramCacheLocker.lock();
            if (this->ramCache.count(cacheName) > 0)
			{
				currentCacheSize -= this->ramCache[cacheName]->size();
                this->ramCache[cacheName]->set(value);
			}
            else
                this->ramCache[cacheName] = new Var(value);


			currentCacheSize += this->ramCache[cacheName]->size();
            this->ramCache[cacheName]->varStateOnFs = DataToBeSet;

			if (this->debugMode)
				cout << "Added the file "<<varName << " to cache"<< endl << flush;
			
			ramCacheLocker.unlock();
        }
        else
        {
			varName = this->getFileName(varName, false);
    		sysLink.waitAndLockFile(varName, 1000);
    		sysLink.writeFile(varName, value);
    		sysLink.unlockFile(varName);

			if (this->debugMode)
				cout << "Writed file"<<varName << endl << flush;
        }
	}

	//set a variable with a double value
	void FileVars::setDouble(string varName, double value)
	{
		this->set(varName, std::to_string(value));
	}

	//set a variable with a long value
	void FileVars::setLong(string varName, long value)
	{
		this->set(varName, std::to_string(value));
	}

	//set a variable with a boolean value
	void FileVars::setBool(string varName, bool value)
	{
		this->set(varName, std::to_string(value));
	}

	//get a varlue of a variable
	Var FileVars::get(string varName, string defaulValue, bool initializeFileAndCahceWithDefaultValue)
	{		
		string originalVarName = varName;
        varName += ".__value__";
		string cacheName = this->getFileName(varName, false);

		if (this->debugMode)
			cout << "Get var "<<varName << endl << flush;
			
		string result = "";

        if ((this->useCache) && (this->containsKey(cacheName)))
        {
            return *this->ramCache[cacheName];
        }
        else
        {

			varName = this->getFileName(varName, false);
    		

			if (this->debugMode)
				cout << "Reading the file"<<varName << endl << flush;
    		if (sysLink.fileExists(varName))
    		{
    			sysLink.waitAndLockFile(varName, 1000);
    			result = sysLink.readFile(varName);
    			sysLink.unlockFile(varName);

				Var * resultVar = new Var(result);
				
                if (this->useCache)
                {
					ramCacheLocker.lock();
					this->ramCache.insert(std::pair<string, Var*>(cacheName, resultVar));
                    //this->ramCache[varName] = new Var(result);
                    this->ramCache[cacheName]->varStateOnFs = AlreadySync;

					int a = this->ramCache.size();
					ramCacheLocker.unlock();
                }

				return *resultVar;
    		}
    		else
			{
				if (initializeFileAndCahceWithDefaultValue)
					this->set(originalVarName, defaulValue);
				
				Var ret(defaulValue);
    			return ret;
			}


    		//string resultVar(result);

    		//return resultVar;
        }
	}

	//delete a variable
	void FileVars::del(string varName)
	{
		string varName2 = varName;
		string cacheName = this->getFileName(varName, false) + "/__value__";

		//delete from cache
		ramCacheLocker.lock();
		if (this->ramCache.find(cacheName) != this->ramCache.end())
        {
			currentCacheSize -= ramCache[cacheName]->size();
			delete ramCache[cacheName];
            this->ramCache.erase(cacheName);
        }
		ramCacheLocker.unlock();

        varName2 += ".__value__";
		varName2 = this->getFileName(varName2, false);
		if (this->debugMode)
			cout << "Del file "<<varName2 << endl << flush;
		sysLink.deleteFile(varName2);

		this->deleteDirectoryTree(this->getFileName(varName, false));
	}

	//gets a var name and return their respective file. This function converts a variable name to a fiilename
	string FileVars::getFileName(string varName, bool autoCreateDirectory)
	{
		for (int cont =0; cont < varName.size(); cont++)
		{
			if (varName[cont] == '.')
				varName[cont] = '/';
		}

		varName = this->directory + varName;

		//removeInvalidchars
		string validChars = "abcdefghijklmnopqrstuvxywzABCDEFGHIJKLMNOPQRSTUVXYWZ_/\\.-+,0123456789";
		string tmp2 = "";
		for (auto &c: varName)
			if (validChars.find(c) != string::npos)
				tmp2 += c;
			else
				tmp2 += "_";

		varName = tmp2;



		if (autoCreateDirectory)
		{
			sysLink.createDirectory(sysLink.getDirectoryName(varName));
		}

		return varName;
	}

	Var FileVars::getPart(string varName, size_t start, size_t count)
	{
		//%%%%% force RAM DUMP 

		//read data to buffer
		char *buffer = new char[count];
        varName += ".__value__";
		sysLink.waitAndLockFile(varName, 1000);
		sysLink.readFile(varName, buffer, start, count);
		sysLink.unlockFile(varName);

		//prepare the result
		Var result(buffer, count);

		//delete the buffer
		delete[] buffer;

		return result;
	}

	size_t FileVars::getVarSize(string varName)
	{
        varName += ".__value__";
		//get real file size
		size_t size = sysLink.getFileSize(varName);
		return size;
	}

	//this function dumps the data stored in RAM to FileSystem
    void FileVars::__Thread_syncToFs()
    {
        string varName;
        while (running)
        {
			ramCacheLocker.lock();
            // for (int cont =0 ; cont < this->ramCache.size(); cont++)
            for (auto const& x : this->ramCache)
            {
                if (x.second->varStateOnFs != AlreadySync)
                {
                    //varName = this->getFileName(x.first, true);
					varName = x.first;
            		sysLink.waitAndLockFile(varName, 1000);

					sysLink.createDirectory(sysLink.getDirectoryName(varName));

					if (x.second->varStateOnFs == DataToBeSet)
					{
            			sysLink.writeFile(varName, x.second->AsString());
						x.second->varStateOnFs = AlreadySync;
					}
					else
					{
						sysLink.appendFile(varName, x.second->AsString());
						//must remove variable from cache (because data can be not equals to file)
						this->ramCache.erase(x.first);
					}

            		sysLink.unlockFile(varName);

					if (this->debugMode)
						cout << "Writed (sync cache) file"<<varName << endl << flush;
                }
            }
			ramCacheLocker.unlock();

			//limits the size of RAM cache
			int count = 0;
			auto initialCacheSize = currentCacheSize;
			ramCacheLocker.lock();

			vector<string> toErase;
			for (auto &x : this->ramCache)
			{
				//ensures the current cache item is already sync to fs and can be deleted
				if (x.second->varStateOnFs == AlreadySync)
				{
					if (currentCacheSize <= this->maxCacheSize)
						break;

					currentCacheSize -= x.second->size();
					x.second->set("");
					auto tmpObject = x.second;
					delete tmpObject;
					toErase.push_back(x.first);
				}
			}
			for (auto &c: toErase)
				this->ramCache.erase(c);
				
			ramCacheLocker.unlock();

			if (this->debugMode && initialCacheSize != currentCacheSize)
				cout << "Released " << to_string(initialCacheSize - currentCacheSize) << " bytes from RAM cache" << endl;

            sysLink.sleep_ms(10);
        }
    }

	vector<string> FileVars::getChilds(string parentName)
	{
		parentName = this->getFileName(parentName, false);

		return sysLink.getDirectoriesFromDirectory(parentName, "");
	}

	bool FileVars::containsKey(string key)
	{
		return this->ramCache.count(key);

		return false;
	}

	void FileVars::deleteDirectoryTree(string directory)
	{
		if (directory.find(this->directory) == 0)
		{
			auto dirExists = sysLink.directoryExists(directory);
			auto directoryObjects = sysLink.getObjectsFromDirectory(directory, "", "-e \"^-\" -e \"^d\"");
			if (dirExists && directoryObjects.size() == 0)
			{
				sysLink.deleteDirectory(directory);
				if (directory.find('/') != string::npos)
				{
					auto parentDir = directory.substr(0, directory.find_last_of('/'));
					deleteDirectoryTree(parentDir);
				}
			}
		}
	}




	//#region Var class
	Var::Var(string data)
	{
		this->_data = data;
	}
	Var::Var(char* buffer, size_t size)
	{
		this->_data = string(buffer, size);
	}

	string Var::AsString()
	{
		return this->_data;
	}

	double Var::AsDouble()
	{
		return std::stod(this->_data);
	}

	long Var::AsLong()
	{
		return std::stol(this->_data);
	}

	bool Var::AsBool()
	{
		string matches = "1trueTrueTRUE";
		return matches.find(this->_data) != string::npos;
	}

    void Var::set(string newData)
    {
        this->_data = newData;
    }

	void Var::append(string newData)
	{
		this->_data += newData;
	}

	size_t Var::size(){
		
		return this->_data.size() + sizeof(this);
	}
	//#endregion

	
}
