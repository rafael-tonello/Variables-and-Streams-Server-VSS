#ifndef __ILOGGERMOCK__H__
#define __ILOGGERMOCK__H__ 

#include <ilogger.h>

class ILoggerMock: public ILogger{
private:
    //start time (when class was initilized)
    time_t startTime;
    bool printByDefault = false;
public:
    void log(int level, string name, string msg) override;
    void trace(string name, string msg) override;
    void debug2(string name, string msg) override;
    void debug(string name, string msg) override;
    void info2(string name, string msg) override;
    void info(string name, string msg) override;
    void warning(string name, string msg) override;
    void error(string name, string msg) override;
    void critical(string name, string msg) override;

    //algo use other overrides implemented in ILogger (all will redirects to the methods above)
    using ILogger::log;
    using ILogger::trace;
    using ILogger::debug2;
    using ILogger::debug;
    using ILogger::info2;
    using ILogger::info;
    using ILogger::warning;
    using ILogger::error;
    using ILogger::critical;

    using ILogger::getNamedLogger;
    using ILogger::getNamedLoggerP;

    int64_t getMsElasped(){
        return (int64_t)(time(0) - startTime)*1000;
    }

    typedef function<void(int level, string name, string msg)> logFunc;
    logFunc logF = [this](int level, string name, string msg){
        if (printByDefault){
            printf("[log instance + %lld ms][%s][%s] %s", getMsElasped(), this->levelToString(level).c_str(),  name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> traceFunc;
    traceFunc traceF = [this](string name, string msg){
        if (printByDefault){
            printf("[trace instance + %lld ms][TRACE][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> debug2Func;
    debug2Func debug2F = [this](string name, string msg){
        if (printByDefault){
            printf("[debug2 instance + %lld ms][DEBUG2][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> debugFunc;
    debugFunc debugF = [this](string name, string msg){
        if (printByDefault){
            printf("[debug instance + %lld ms][DEBUG][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> info2Func;
    info2Func info2F = [this](string name, string msg){
        if (printByDefault){
            printf("[info2 instance + %lld ms][INFO2][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> infoFunc;
    infoFunc infoF = [this](string name, string msg){
        if (printByDefault){
            printf("[info instance + %lld ms][INFO][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> warningFunc;
    warningFunc warningF = [this](string name, string msg){
        if (printByDefault){
            printf("[warning instance + %lld ms][WARNING][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> errorFunc;
    errorFunc errorF = [this](string name, string msg){
        if (printByDefault){
            printf("[error instance + %lld ms][ERROR][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

    typedef function<void(string name, string msg)> criticalFunc;
    criticalFunc criticalF = [this](string name, string msg){
        if (printByDefault){
            printf("[critical instance + %lld ms][CRITICAL][%s] %s", getMsElasped(), name.c_str(), msg.c_str());
        }   
    };

public:
    
    void setOnLogFunc(logFunc f){ logF = f; }
    void setOnTraceFunc(traceFunc f){ traceF = f; }
    void setOnDebug2Func(debug2Func f){ debug2F = f; }
    void setOnDebugFunc(debugFunc f){ debugF = f; }
    void setOnInfo2Func(info2Func f){ info2F = f; }
    void setOnInfoFunc(infoFunc f){ infoF = f; }
    void setOnWarningFunc(warningFunc f){ warningF = f; }
    void setOnErrorFunc(errorFunc f){ errorF = f; }
    void setOnCriticalFunc(criticalFunc f){ criticalF = f; }

    ILoggerMock(bool defaultBehaviourIsPrint = false);
    ~ILoggerMock();
};

#endif
