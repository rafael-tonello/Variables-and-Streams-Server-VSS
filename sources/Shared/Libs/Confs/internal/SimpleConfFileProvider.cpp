#include "SimpleConfFileProvider.h"

Shared::SimpleConfFileProvider::SimpleConfFileProvider(string filename)
{
    this->filename = filename;
    this->running = true;
    //std::async(std::launch::async, [&](){ this->fileCheckPoll(); } );
    fileCheckThread = std::thread([=]() { this->fileCheckPoll(); });
}


Shared::SimpleConfFileProvider::~SimpleConfFileProvider()
{
    running = false;
    if (fileCheckThread.joinable())
        fileCheckThread.join();
}

void Shared::SimpleConfFileProvider::fileCheckPoll()
{
    auto lastChangeTime = getFileChangeTime(filename);
    while (running)
    {
        auto currChangeTime = getFileChangeTime(filename);
        if (currChangeTime != lastChangeTime)
        {
            lastChangeTime = currChangeTime;
            readAndNotify();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

long int Shared::SimpleConfFileProvider::getFileChangeTime(string fname)
{
    struct stat result;
    if (stat(fname.c_str(), &result)==0)
        return result.st_mtim.tv_nsec;

    return 0;
}
    
vector<tuple<string, string>> Shared::SimpleConfFileProvider::readAllConfigurations()
{
    ifstream fileStream(this->filename);
    vector<tuple<string, string>> result;

    for(string line; getline(fileStream, line); )
    {
        line = ltrim(line);
        //remove \r and \n from line (if exists)
        if (line.size() > 0 && line[line.size()-1] == '\n')
            line = line.substr(0, line.size()-1);
            
        if (line.size() > 0 && line[line.size()-1] == '\r')
            line = line.substr(0, line.size()-1);

        if (line != "" && line[0] != '#' && line[0] != '/' && line[0] != ';')
        {
            //try detect the separator from current line
            string lineSep = this->identifyKeyValueSeparator(line);
            if (lineSep != "")
            {
                string name = rtrim(line.substr(0, line.find(lineSep)));
                string value = rtrim(ltrim(line.substr(line.find(lineSep)+1)));

                result.push_back({name, value});
            }
            else
            {
                //trey use the first space as a charcter separator

            }
        }
    }

    return result;
}

void Shared::SimpleConfFileProvider::listen(function<void(string, DynamicVar)> onData)
{
    this->_onData = onData;
    readAndNotify();
}

string Shared::SimpleConfFileProvider::ltrim(string str)
{
    size_t c = 0;
    string to_remove = " \t";
    for (; c < str.size(); c++)
        if (to_remove.find(str[c]) == string::npos)
            break;

    if (c < str.size())
        return str.substr(c);
    
    return "";
}

string Shared::SimpleConfFileProvider::rtrim(string str)
{
    string to_remove = " \t";
    size_t c = str.size()-1;
    for (; c >= 0 ; c--)
        if (to_remove.find(str[c]) == string::npos)
            break;

    if (c > 0)
        return str.substr(0, c+1);
    
    return "";

}


string Shared::SimpleConfFileProvider::identifyKeyValueSeparator(string str)
{
    for (size_t c = 0; c < str.size(); c++)
        if (separatorChars.find(str[c]) != string::npos)
        {
            char sep = str[c];
            string ret;
            ret.push_back(sep);
            return ret;
        }

    return "";
}

void Shared::SimpleConfFileProvider::readAndNotify()
{
    auto confs = this->readAllConfigurations();
    for (auto &c: confs)
    {
        string key = std::get<0>(c);
        DynamicVar value = std::get<1>(c);
        


        if (!currValues.count(key) || !currValues[key].isEquals(value))
            _onData(key, value);

        currValues[key] = value;
    }
}

bool Shared::SimpleConfFileProvider::contains(string name)
{
    return currValues.count(name);
}

DynamicVar Shared::SimpleConfFileProvider::get(string name)
{
    if (currValues.count(name))
        return currValues[name];

    return "";
}
