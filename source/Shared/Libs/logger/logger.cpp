#include  "logger.h" 

int Logger::LOGGER_DEBUG_LEVEL = 0;
int Logger::LOGGER_INFO_LEVEL = 1;
int Logger::LOGGER_WARNING_LEVEL = 2;
int Logger::LOGGER_ERROR_LEVEL = 3;
int Logger::LOGGER_CRITICAL_LEVEL = 4;

Logger::Logger(vector<ILogWriter*> writers, int logLevel = LOGGER_INFO_LEVEL){
    this->writers = writers;
    this->logLevel = logLevel;
}

void Logger::log(string msg, int level, string name="")
{
    for (auto &c : this->writers)
        c->write(this, msg, level, name, level >= logLevel);
}

void Logger::debug(string msg, string name=""){
    log(msg, LOGGER_DEBUG_LEVEL, name);
}

void Logger::Logger::info(string msg, string name=""){
    log(msg, LOGGER_INFO_LEVEL, name);
}

void Logger::warning(string msg, string name=""){
    log(msg, LOGGER_WARNING_LEVEL, name);
}

void Logger::error(string msg, string name=""){
    log(msg, LOGGER_ERROR_LEVEL, name);
}

void Logger::critical(string msg, string name="", bool raiseException = true){
    log(msg, LOGGER_CRITICAL_LEVEL, name);
    if (raiseException)
        throw runtime_error(msg.c_str());
}