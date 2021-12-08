#include  "LoggerFileWriter.h" 
 
LoggerFileWriter::LoggerFileWriter(string fname)
{
    file = ofstream(fname);
    if (!file.is_open())
        throw runtime_error("Can't open log file to write");
    
}
void LoggerFileWriter::write(Logger* sender, string msg, int level, string name, bool aboveOrInLogLevel){
    file << (name != "" ? "["+ name + "]" : "") << msg << endl;
}