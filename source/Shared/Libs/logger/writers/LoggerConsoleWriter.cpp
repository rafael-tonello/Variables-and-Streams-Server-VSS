#include  "LoggerConsoleWriter.h" 
 
LoggerConsoleWriter::LoggerConsoleWriter(bool useColors)
{
    this->colors = useColors;
}
 
void LoggerConsoleWriter::write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel){
    if (aboveOrInLogLevel)
    {
        if (level == Logger::LOGGER_WARNING_LEVEL)
            cout << "\e[0;33m        " << msg << "\e[0m" << endl; 
        else if (level == sender->Logger::LOGGER_ERROR_LEVEL)
            cout << "\e[0;31m        " << msg << "\e[0m" << endl;
        else if (level == sender->Logger::LOGGER_CRITICAL_LEVEL)
            cout << "\e[0;31m        " << msg << "\e[0m" << endl;
        //else if (level == sender->infoLevel)	
        //	cout << "\e[0;34m        " << msg << "\e[0m" << endl;
        else
            cout << msg << endl;
        
    }
}