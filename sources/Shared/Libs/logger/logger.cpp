#include  "logger.h" 

int Logger::LOGGER_DEBUG_LEVEL = 0;
int Logger::LOGGER_INFO_LEVEL = 1;
int Logger::LOGGER_WARNING_LEVEL = 2;
int Logger::LOGGER_ERROR_LEVEL = 3;
int Logger::LOGGER_CRITICAL_LEVEL = 4;

Logger::Logger(vector<ILogWriter*> writers, int logLevel){
    this->writers = writers;
    this->logLevel = logLevel;
}

void Logger::log(string name, string msg, int level)
{
    for (auto &c : this->writers)
        c->write(this, msg, level, name, level >= logLevel);
}

void Logger::debug(string name, string msg){
    log(name, msg, LOGGER_DEBUG_LEVEL);
}

void Logger::Logger::info(string name, string msg){
    log(name, msg, LOGGER_INFO_LEVEL);
}

void Logger::warning(string name, string msg){
    log(name, msg, LOGGER_WARNING_LEVEL);
}

void Logger::error(string name, string msg){
    log(name, msg, LOGGER_ERROR_LEVEL);
}

void Logger::critical(string name, string msg, bool raiseException){
    log(name, msg, LOGGER_CRITICAL_LEVEL);
    if (raiseException)
        throw runtime_error(msg.c_str());
}

void Logger::log_v(string name, vector<DynamicVar> msgs, int level)
{
    this->log(name, fromList(msgs), level);
}

void Logger::debug_v(string name, vector<DynamicVar> msgs)
{
    this->debug(name, fromList(msgs));
}

void Logger::info_v(string name, vector<DynamicVar> msgs)
{
    this->info(name, fromList(msgs));
}

void Logger::warning_v(string name, vector<DynamicVar> msgs)
{
    this->warning(name, fromList(msgs));
}

void Logger::error_v(string name, vector<DynamicVar> msgs)
{
    this->error(name, fromList(msgs));
}

void Logger::critical_v(string name, vector<DynamicVar> msgs, bool raiseException)
{
    this->critical(name, fromList(msgs), raiseException);

}
