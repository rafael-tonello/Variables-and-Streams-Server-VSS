#include  "LoggerFileWriter.h" 
 
LoggerFileWriter::LoggerFileWriter(string fname, bool writeDate, bool writeTime)
{
    _writeDate = writeDate;
    _writeTime = writeTime;
    file = ofstream(fname);
    if (!file.is_open())
        throw runtime_error("Can't open log file to write");
    
}

LoggerFileWriter::~LoggerFileWriter()
{
    file.close();
}

void LoggerFileWriter::write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel){

    string dateTime = generateDateTimeString();

    string prefix = "[";

    if (dateTime != "")
    {
        prefix += dateTime;
    }

    if (name != "")
    {
        if (prefix.size() > 1)
            prefix +=", ";

        prefix += name;
    }

    prefix += "] ";

    if (prefix.size() <= 3)
        prefix = "";

    file << prefix << msg << endl;
}

string LoggerFileWriter::getDate()
{

    std::time_t rawtime;
    std::tm* timeinfo;
    char buffer [80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);

    std::strftime(buffer,80,"%Y-%m-%d",timeinfo);

    return string(buffer);
}

string LoggerFileWriter::getTime()
{
    std::time_t rawtime;
    std::tm* timeinfo;
    char buffer [80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);

    std::strftime(buffer,80,"%H-%M-%S",timeinfo);

    return string(buffer);

}

string LoggerFileWriter::generateDateTimeString()
{

    string result = "";

    if (_writeDate)
        result = getDate();

    if (_writeTime)
    {
        string time = getTime();
        if (time != "")
        {
            if (result != "")
                result += "T";

            result += time;
        }
    }

    return result;
}
