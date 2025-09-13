#include "iloggermock.h"

ILoggerMock::ILoggerMock(bool  defaultBehaviourIsPrint )
{
    startTime = time(0);
    printByDefault = defaultBehaviourIsPrint;
}

ILoggerMock::~ILoggerMock()
{
    
}

void ILoggerMock::log(int level, string name, string msg)
{
    logF(level, name, msg);
}

void ILoggerMock::trace(string name, string msg)
{
    traceF(name, msg);
}

void ILoggerMock::debug2(string name, string msg)
{
    debug2F(name, msg);
}

void ILoggerMock::debug(string name, string msg)
{
    debugF(name, msg);
}

void ILoggerMock::info2(string name, string msg)
{
    info2F(name, msg);
}

void ILoggerMock::info(string name, string msg)
{
    infoF(name, msg);
}

void ILoggerMock::warning(string name, string msg)
{
    warningF(name, msg);
}

void ILoggerMock::error(string name, string msg)
{
    errorF(name, msg);
}

void ILoggerMock::critical(string name, string msg)
{
    criticalF(name, msg);
}
