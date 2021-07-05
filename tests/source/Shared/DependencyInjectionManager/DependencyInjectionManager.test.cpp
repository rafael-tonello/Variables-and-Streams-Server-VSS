#include "DependencyInjectionManager.test.h"

vector<string> DependencyInjectionManagerTester::getContexts()
{
    return {"Shared.DepencyInjection"};

}



class ITest{
    public: 
        virtual void test() = 0;
        bool testCalled = false;
        int aCounter = 0;
};

bool __test1__destroyed = false;
class Test1: public ITest{
    public:
        ~Test1(){
            __test1__destroyed = true;
        }

        void test(){
            testCalled = true;
        }
};

class Test2: public ITest{
    private:
        ITest *childTest;
    public:
        Test2(DependencyInjectionManager *di){
            childTest = di->get<ITest>("test1instance");
        }
        void test(){
            testCalled = true;
            childTest->test();
        }
};

void DependencyInjectionManagerTester::run(string context)
{
    if (context != "Shared.DepencyInjection")
        return;

    /*
    void addSingleton(string name, function<void*()> createInstance, bool instantiateImediately);
	void addSingleton(string name, T* instance);
	void addSingleton(string name, function<T*()> createInstance);
	void addMultiInstance(string name, function<T*()> createInstance);
	T* get(string name);
	void putBack(string name, T* instance);
    */
        
    DependencyInjectionManager di;
    
    this->test("test1instance.testCalled should be true and test2instance.testCalled should be false", [&](){
        di.addSingleton<ITest>("test1instance", new Test1());
        di.addSingleton<ITest>("test2instance", new Test2(&di));

        di.get<ITest>("test1instance")->test();
        string r = di.get<ITest>("test1instance")->testCalled == true? "true":"false";
        r+= ", ";
        r += di.get<ITest>("test2instance")->testCalled == true? "true":"false";

        return TestResult{
            r == "true, false",
            "true, false",
            r
        };
        
    });

    DependencyInjectionManager di2;
    
    this->test("test1instance.testCalled and test2instance.testCalled should be true", [&](){
        di2.addSingleton<ITest>("test1instance", new Test1());
        di2.addSingleton<ITest>("test2instance", new Test2(&di2));

        di2.get<ITest>("test2instance")->test();
        string r = di2.get<ITest>("test1instance")->testCalled == true? "true":"false";
        r+= ", ";
        r += di2.get<ITest>("test2instance")->testCalled == true? "true":"false";

        return TestResult{
            r == "true, true",
            "true, true",
            r
        };
        
    });
    
    //test pre instantiated singleton

    this->test("Test1.aCount should be 5", [&](){
        di.get<ITest>("test1instance")->aCounter++;
        di.get<ITest>("test1instance")->aCounter++;
        di.get<ITest>("test1instance")->aCounter++;
        di.get<ITest>("test1instance")->aCounter++;
        di.get<ITest>("test1instance")->aCounter++;

        return TestResult{
            di.get<ITest>("test1instance")->aCounter == 5,
            "5",
            std::to_string(di.get<ITest>("test1instance")->aCounter)
        };
        
    });
    
    

    this->test("test1multinstance.aCount should be 5", [&](){
        di.addMultiInstance<ITest>("test1multinstance", [](){
            return new Test1();
        });

        di.get<ITest>("test1multinstance")->aCounter++;
        di.get<ITest>("test1multinstance")->aCounter++;
        di.get<ITest>("test1multinstance")->aCounter++;
        di.get<ITest>("test1multinstance")->aCounter++;
        di.get<ITest>("test1multinstance")->aCounter++;
        di.get<ITest>("test1multinstance")->aCounter++;

        return TestResult{
            di.get<ITest>("test1multinstance")->aCounter == 0,
            "test1multinstance",
            std::to_string(di.get<ITest>("test1multinstance")->aCounter)
        };
        
    });

    //test onDemand singleton
    int test1OnDemandInstances = 0;
    di.addSingleton<ITest>("test1onDemandSingleton", [&]{
        test1OnDemandInstances++;
        return (ITest*)(new Test1());
    });

    this->test("test1onDemandSingleton should be NULL before the first get", [&](){

        ITest *instance = NULL;

        if (di.singletons.count("test1onDemandSingleton") != 0)
            instance = (ITest*)di.singletons["test1onDemandSingleton"].instance;

        return TestResult{
            instance == NULL,
            "NULL",
            instance == NULL ? "NULL" : "&instance="+std::to_string((uint64_t)instance)
        };
        
    });

    this->test("test1onDemandSingleton should be valid after the first get", [&](){

        ITest *instance = NULL;

        di.get<ITest>("test1onDemandSingleton")->aCounter = 10;

        if (di.singletons.count("test1onDemandSingleton") != 0)
            instance = (ITest*)di.singletons["test1onDemandSingleton"].instance;

        return TestResult{
            instance != NULL,
            "&intance=[a valid address]",
            instance == NULL ? "NULL" : "&instance="+std::to_string((uint64_t)instance)
        };
    });

    this->test("test1onDemandSingleton should only one instance", [&](){

        di.get<ITest>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest>("test1onDemandSingleton")->aCounter = 10;

        
        return TestResult{
            test1OnDemandInstances = 1,
            "1",
            std::to_string(test1OnDemandInstances)
        };
    });

    

}



//teste on demand instances

//test destructor of singletons

//test destructor of on demand instances
