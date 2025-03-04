#ifndef __RAMCACHEDBTEST__H__ 
#define __RAMCACHEDBTEST__H__ 

#include <tester.h>
#include <dependencyInjectionManager.h>
#include <logger.h>
#include <LoggerLambdaWriter.h>
#include <Confs.h>
#include <IConfProvider.h>
#include <InMemoryConfProvider.h>
#include <ramcachedb.h>

class RamCacheDBTest: public Tester{
public:
    RamCacheDBTest();
    ~RamCacheDBTest();
public: 
    /** Tester interface */
    vector<string> getContexts() override;
    void run(string context) override;
};

#endif
