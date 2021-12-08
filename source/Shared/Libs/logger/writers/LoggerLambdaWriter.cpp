#include  "LoggerLambdaWriter.h" 
 
LoggerLambdaWriter::LoggerLambdaWriter(function<void(Logger* sender, string msg, int level, bool aboveOrInLogLevel)> writerFunc)
{
    this->writerFunc = writerFunc;
}
	
void LoggerLambdaWriter::write(Logger* sender, string msg, int level, bool aboveOrInLogLevel)
{
    this->writerFunc(sender, msg, level, aboveOrInLogLevel);
}