#include "DependencyInjectionManager.test.h"

vector<string> DependencyInjectionManagerTester::getContexts()
{
    return {"Shared.DependencyInjection"};

}





void DependencyInjectionManagerTester::run(string context)
{
    if (context != "Shared.DependencyInjection")
        return;

    /*
    void addSingleton(string name, function<void*()> createInstance, bool instantiateImediately);
	void addSingleton(string name, T* instance);
	void addSingleton(string name, function<T*()> createInstance);
	void addMultiInstance(string name, function<T*()> createInstance);
	T* get(string name);
	void putBack(string name, T* instance);
    */

   this->yellowMessage("Tests using types");
   this->testsUsingTypes();
   this->yellowMessage("Tests using names");
   this->testsUsingNames();

}

class ITest1_types{
    public: 
        virtual void test() = 0;
        bool testCalled = false;
        int aCounter = 0;
};

class ITest2_types{
    public: 
        virtual void test() = 0;
        bool testCalled = false;
        int aCounter = 0;
};



bool __test1__destroyed_types = false;

class Test1_types: public ITest1_types{
    public:
        ~Test1_types(){
            __test1__destroyed_types = true;
        }

        void test(){
            testCalled = true;
        }
};

class Test2_types: public ITest2_types{
    private:
        ITest1_types *childTest;
    public:
        Test2_types(DependencyInjectionManager *di){
            childTest = di->get<ITest1_types>();
        }
        void test(){
            testCalled = true;
            childTest->test();
        }
};


class ITest3_types{public: int aCounter = 0; };
class ITest4_types{public: int aCounter = 0; };
class ITest5_types{public: int aCounter = 0; };
class Test3_types: public ITest3_types{};
class Test4_types: public ITest4_types{};
class Test5_types: public ITest5_types{};

void DependencyInjectionManagerTester::testsUsingTypes(){
        
    DependencyInjectionManager di;
    
    this->test("Test1.testCalled should be true and Test2.testCalled should be false", [&](){
        di.addSingleton<ITest1_types>(new Test1_types());
        di.addSingleton<ITest2_types>(new Test2_types(&di));

        di.get<ITest1_types>()->test();
        string r = di.get<ITest1_types>()->testCalled == true? "true":"false";
        r+= ", ";
        r += di.get<ITest2_types>()->testCalled == true? "true":"false";

        return TestResult{
            r == "true, false",
            "true, false",
            r
        };
        
    });

    DependencyInjectionManager di2;
    
    this->test("Test1.testCalled and Test2.testCalled should be true", [&](){
        di2.addSingleton<ITest1_types>(new Test1_types());
        di2.addSingleton<ITest2_types>(new Test2_types(&di2));

        di2.get<ITest2_types>()->test();
        string r = di2.get<ITest1_types>()->testCalled == true? "true":"false";
        r+= ", ";
        r += di2.get<ITest2_types>()->testCalled == true? "true":"false";

        return TestResult{
            r == "true, true",
            "true, true",
            r
        };
        
    });

    //test pre instantiated singleton
    this->test("Test3.aCount should be 5", [&](){
        di.addSingleton<ITest3_types>(new Test3_types());

        di.get<ITest3_types>()->aCounter++;
        di.get<ITest3_types>()->aCounter++;
        di.get<ITest3_types>()->aCounter++;
        di.get<ITest3_types>()->aCounter++;
        di.get<ITest3_types>()->aCounter++;

        return TestResult{
            di.get<ITest3_types>()->aCounter == 5,
            "5",
            std::to_string(di.get<ITest3_types>()->aCounter)
        };
        
    });

    this->test("Test4.aCount should be 0", [&](){
        di.addMultiInstance<ITest4_types>([](){
            return new Test4_types();
        });

        di.get<ITest4_types>()->aCounter++;
        di.get<ITest4_types>()->aCounter++;
        di.get<ITest4_types>()->aCounter++;
        di.get<ITest4_types>()->aCounter++;
        di.get<ITest4_types>()->aCounter++;
        di.get<ITest4_types>()->aCounter++;

        auto resultObject = di.get<ITest4_types>();
        int resultValue = resultObject->aCounter;
        string resultValueStr = std::to_string(resultValue);

        return TestResult{
            resultValue == 0,
            "0",
            resultValueStr
        };
        
    });

    int test5OnDemandInstances_names = 0;
    di.addSingleton<ITest5_types>([&]{
        test5OnDemandInstances_names++;
        return (ITest5_types*)(new Test5_types());
    });

    this->test("ITest5 should be NULL before the first get", [&](){

        ITest5_types *instance = NULL;

        for (auto &c: di.singletons)
        {
            if (c.typesAndNames.find(typeid(ITest5_types).name()) != string::npos)
                instance = (ITest5_types*)c.instance;
        }

        return TestResult{
            instance == NULL,
            "NULL",
            instance == NULL ? "NULL" : "&instance="+std::to_string((uint64_t)instance)
        };
        
    });

    this->test("ITest5 should be valid after the first get", [&](){

        ITest5_types *instance = NULL;

        di.get<ITest5_types>()->aCounter = 10;

        for (auto &c: di.singletons)
        {
            if (c.typesAndNames.find(typeid(ITest5_types).name()) != string::npos)
                instance = (ITest5_types*)c.instance;
        }

        return TestResult{
            instance != NULL,
            "&intance=[a valid address]",
            instance == NULL ? "NULL" : "&instance="+std::to_string((uint64_t)instance)
        };
    });

    this->test("ITest5_types(singleton mode) should only have one instance", [&](){

        di.get<ITest5_types>()->aCounter = 10;
        di.get<ITest5_types>()->aCounter = 10;
        di.get<ITest5_types>()->aCounter = 10;
        di.get<ITest5_types>()->aCounter = 10;
        di.get<ITest5_types>()->aCounter = 10;

        
        return TestResult{
            test5OnDemandInstances_names == 1,
            "1",
            std::to_string(test5OnDemandInstances_names)
        };
    });

}


class ITest_names{
    public: 
        virtual void test() = 0;
        bool testCalled = false;
        int aCounter = 0;
};

bool __test1__destroyed = false;
class Test1_names: public ITest_names{
    public:
        ~Test1_names(){
            __test1__destroyed = true;
        }

        void test(){
            testCalled = true;
        }
};

class Test2_names: public ITest_names{
    private:
        ITest_names *childTest;
    public:
        Test2_names(DependencyInjectionManager *di){
            childTest = di->get<ITest_names>("test1instance");
        }
        void test(){
            testCalled = true;
            childTest->test();
        }
};

void DependencyInjectionManagerTester::testsUsingNames(){
        
    DependencyInjectionManager di;
    
    this->test("test1instance.testCalled should be true and test2instance.testCalled should be false", [&](){
        di.addSingleton<ITest_names>(new Test1_names(), {"test1instance"});
        di.addSingleton<ITest_names>(new Test2_names(&di), {"test2instance"});

        di.get<ITest_names>("test1instance")->test();
        string r = di.get<ITest_names>("test1instance")->testCalled == true? "true":"false";
        r+= ", ";
        r += di.get<ITest_names>("test2instance")->testCalled == true? "true":"false";

        return TestResult{
            r == "true, false",
            "true, false",
            r
        };
        
    });

    DependencyInjectionManager di2;
    
    this->test("test1instance.testCalled and test2instance.testCalled should be true", [&](){
        di2.addSingleton<ITest_names>(new Test1_names(), {"test1instance"});
        di2.addSingleton<ITest_names>(new Test2_names(&di2), {"test2instance"});

        di2.get<ITest_names>("test2instance")->test();
        string r = di2.get<ITest_names>("test1instance")->testCalled == true? "true":"false";
        r+= ", ";
        r += di2.get<ITest_names>("test2instance")->testCalled == true? "true":"false";

        return TestResult{
            r == "true, true",
            "true, true",
            r
        };
        
    });
    
    //test pre instantiated singleton

    this->test("Test1.aCount should be 5", [&](){
        di.get<ITest_names>("test1instance")->aCounter++;
        di.get<ITest_names>("test1instance")->aCounter++;
        di.get<ITest_names>("test1instance")->aCounter++;
        di.get<ITest_names>("test1instance")->aCounter++;
        di.get<ITest_names>("test1instance")->aCounter++;

        return TestResult{
            di.get<ITest_names>("test1instance")->aCounter == 5,
            "5",
            std::to_string(di.get<ITest_names>("test1instance")->aCounter)
        };
        
    });
    
    

    this->test("test1multinstance.aCount should be 0", [&](){
        di.addMultiInstance<ITest_names>([](){
            return new Test1_names();
        }, {"test1multinstance"});

        di.get<ITest_names>("test1multinstance")->aCounter++;
        di.get<ITest_names>("test1multinstance")->aCounter++;
        di.get<ITest_names>("test1multinstance")->aCounter++;
        di.get<ITest_names>("test1multinstance")->aCounter++;
        di.get<ITest_names>("test1multinstance")->aCounter++;
        di.get<ITest_names>("test1multinstance")->aCounter++;

        return TestResult{
            di.get<ITest_names>("test1multinstance")->aCounter == 0,
            "0",
            std::to_string(di.get<ITest_names>("test1multinstance")->aCounter)
        };
        
    });

    //test onDemand singleton
    int test1OnDemandInstances_names = 0;
    di.addSingleton<ITest_names>([&]{
        test1OnDemandInstances_names++;
        return (ITest_names*)(new Test1_names());
    }, {"test1onDemandSingleton"});

    this->test("test1onDemandSingleton should be NULL before the first get", [&](){

        ITest_names *instance = NULL;

        for (auto &c: di.singletons)
            if (c.typesAndNames.find("test1onDemandSingleton") != string::npos)
                instance = (ITest_names*)c.instance;

        return TestResult{
            instance == NULL,
            "NULL",
            instance == NULL ? "NULL" : "&instance="+std::to_string((uint64_t)instance)
        };
        
    });

    this->test("test1onDemandSingleton should be valid after the first get", [&](){

        ITest_names *instance = NULL;

        di.get<ITest_names>("test1onDemandSingleton")->aCounter = 10;

        for (auto &c: di.singletons)
            if (c.typesAndNames.find("test1onDemandSingleton") != string::npos)
                instance = (ITest_names*)c.instance;

        return TestResult{
            instance != NULL,
            "&intance=[a valid address]",
            instance == NULL ? "NULL" : "&instance="+std::to_string((uint64_t)instance)
        };
    });

    this->test("test1onDemandSingleton should only one instance", [&](){

        di.get<ITest_names>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest_names>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest_names>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest_names>("test1onDemandSingleton")->aCounter = 10;
        di.get<ITest_names>("test1onDemandSingleton")->aCounter = 10;

        
        return TestResult{
            test1OnDemandInstances_names == 1,
            "1",
            std::to_string(test1OnDemandInstances_names)
        };
    });

    

}



//teste on demand instances

//test destructor of singletons

//test destructor of on demand instances
