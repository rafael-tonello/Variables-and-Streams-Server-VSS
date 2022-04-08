#include "Controller.test.h"

ControllerTester::ControllerTester()
{
    dim.addSingleton<ILogger>(new Logger({new LoggerConsoleWriter(true)}));
    dim.addSingleton<ThreadPool>(new ThreadPool(4));
    dim.addSingleton<Shared::Config>(new Shared::Config(getConfigurationProvider()), {typeid(Shared::Config).name()});
    dim.addSingleton<StorageInterface>(new TmpDBInMemory());
}


vector<string> ControllerTester::getContexts()
{
    return {"Controller"};
}

void ControllerTester::run(string context)
{
    if (context == "Controller")
    {
        test_function__createUniqueId();
        test_function_updateClientAboutObservatingVars();
        test_function_notifyClient();
        test_function_deleteClient();
        test_function_checkClientLiveTime();
        test_function_internalSetVar();
        test_function_getVarInternalFlag();
        test_function_setVarInternalFlag();
        test_function_notifyVarModification();
        test_function_notifyParentGenericObservers();
        test_function_notifyClientsAboutVarChange();
        test_function_apiStarted();
        test_function_clientConnected();
        test_function_observeVar();
        test_function_stopObservingVar();
        test_function_getVar();
        test_function_setVar();
        test_function_delVar();
        test_function_getChildsOfVar();
        test_function_lockVar();
        test_function_unlockVar();       
    }
}

void ControllerTester::test_function__createUniqueId()
{
    this->test("_createUniqueId() sould return different ids between calls", [&](){
        auto id1 = ctrl->_createUniqueId();
        auto id2 = ctrl->_createUniqueId();

        return id1 != id2;
    });
}

void ControllerTester::test_function_updateClientAboutObservatingVars()
{

}

void ControllerTester::test_function_notifyClient()
{

}

void ControllerTester::test_function_deleteClient()
{

}

void ControllerTester::test_function_checkClientLiveTime()
{

}

void ControllerTester::test_function_internalSetVar()
{

}

void ControllerTester::test_function_getVarInternalFlag()
{

}

void ControllerTester::test_function_setVarInternalFlag()
{

}

void ControllerTester::test_function_notifyVarModification()
{

}

void ControllerTester::test_function_notifyParentGenericObservers()
{

}

void ControllerTester::test_function_notifyClientsAboutVarChange()
{

}

void ControllerTester::test_function_apiStarted()
{

}

void ControllerTester::test_function_clientConnected()
{

}

void ControllerTester::test_function_observeVar()
{

}

void ControllerTester::test_function_stopObservingVar()
{

}

void ControllerTester::test_function_getVar()
{

}

void ControllerTester::test_function_setVar()
{

}

void ControllerTester::test_function_delVar()
{

}

void ControllerTester::test_function_getChildsOfVar()
{

}

void ControllerTester::test_function_lockVar()
{

}

void ControllerTester::test_function_unlockVar()
{

}

shared_ptr<IConfigurationProvider> ControllerTester::getConfigurationProvider()
{
    shared_ptr<IConfigurationProvider> confProvider = shared_ptr<IConfigurationProvider>(
        new InMemoryConfProvider(
            {
                std::make_tuple("sample", "value")
            }
        )
    );

    return confProvider;
}