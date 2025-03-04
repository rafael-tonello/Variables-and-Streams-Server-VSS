#include "tester.h"

string Tester::global_test_result = "";
uint Tester::testsFailed = 0;
uint Tester::testsPassed = 0;
mutex Tester::msgBusObserversLocker;
vector<ObserverAndFilter> Tester::msgBusObservers;

void Tester::setTag(string tag, string value){tags[tag] = value;}
string Tester::getTag(string tag, string defValue){if (tags.count(tag) > 0) return tags[tag]; else return defValue;}

void Tester::errorMessage(string message){ cerr << "\e[0;31m        " << "'"<< message << "\e[0m" << endl; }

void Tester::redMessage(string message){ cout << "\e[0;31m        " << message << "\e[0m" << endl; }
void Tester::greenMessage(string message){ cout << "\e[0;32m        " << message << "\e[0m" << endl; }
void Tester::yellowMessage(string message){ cout << "\e[0;33m        " << message << "\e[0m" << endl; }
void Tester::blueMessage(string message){ cout << "\e[0;34m        " << message << "\e[0m" << endl; }
void Tester::cyanMessage(string message){ cout << "\e[0;96m        " << message << "\e[0m" << endl; }
void Tester::brightMessage(string message){ cout << "\e[0;97m        " << message << "\e[0m" << endl; }

void Tester::disableStdout()
{
    std::cout.setstate(std::ios_base::failbit);
    std::cerr.setstate(std::ios_base::failbit);
}

void Tester::enableStdout()
{
    std::cout.clear();
    std::cerr.clear();
}



void Tester::setTestsMessagesPrefix(string prefix){this->testMsgPrefix = prefix;};

void Tester::test(string desc, function<bool()> func, string passMessage, string failMessage){
    try{
        if (func())
        {
            Tester::testsPassed++;
            cout << "\e[0;32m"<< "        " << testMsgPrefix << "[✔]" <<  "'"<< desc <<"' PASSED";
            if (passMessage != "")
                cout << ": " << passMessage;
            cout << "\e[0m" << endl;
        }
        else
        {
            Tester::testsFailed++;

            string message = "        " + string(testMsgPrefix) +string("[✘]") +  string("'") + string(desc) + string("' FAILED");
            if (failMessage != "")
                message += ": " + failMessage;

            cerr << "\e[0;31m" << message << "\033[0m" << endl;
        }
    }
    catch (TestAssertException e)
    {
        Tester::testsFailed++;
        cerr << "\e[0;31m" << "        " << testMsgPrefix << "[✘]" <<  "'"<< desc <<"' FAILED (exception): " << e.what() << "\033[0m" << endl;
    }
    catch(exception e)
    {
        Tester::testsFailed++;
        cerr << "\e[0;31m" << "        " << testMsgPrefix << "[✘]" <<  "'"<< desc <<"' FAILED (exception): " << e.what() << "\033[0m" << endl;
    }
    catch(string e)
    {
        Tester::testsFailed++;
        cerr << "\e[0;31m" << "        " << testMsgPrefix << "[✘]" <<  "'"<< desc <<"' FAILED (exception): " << e << "\033[0m" << endl;
    }
    catch(DynamicVar e)
    {
        Tester::testsFailed++;
        cerr << "\e[0;31m" << "        " << testMsgPrefix << "[✘]" <<  "'"<< desc <<"' FAILED (exception): " << e.getString() << "\033[0m" << endl;
    }
    

    catch (TestAssertException *e)
    {
        Tester::testsFailed++;
        cerr << "\e[0;31m" << "        " << testMsgPrefix << "[✘]" <<  "'"<< desc <<"' FAILED (exception): " << e->what() << "\033[0m" << endl;
        delete e;
    }
    catch(exception *e)
    {
        Tester::testsFailed++;
        cerr << "\e[0;31m" << "        " << testMsgPrefix << "[✘]" <<  "'"<< desc <<"' FAILED (exception): " << e->what() << "\033[0m" << endl;
        delete e;
    }
    catch (...)
    {
        Tester::testsFailed++;
        cerr << "\e[0;31m" << "        " << testMsgPrefix << "[✘]" <<  "'"<< desc <<"' FAILED (exception): Unkown exception" << "\033[0m" << endl;
    }
}

template <class T>
void Tester::test(
    string desc, 
    function<T()> func, 
    T expected, 
    function<bool(T, T)> compareFunc,
    string passMessage,
    string failMessage
)
{
    T r = func();

    test(desc, [&](){
        return compareFunc(r, expected);
    }, passMessage, failMessage);
}

/*void test(
    string desc, 
    function<Shared::DynamicVar()> func, 
    Shared::DynamicVar expected, 
    function<bool(Shared::DynamicVar, Shared::DynamicVar)> cmpFun = [](Shared::DynamicVar returned, Shared::DynamicVar expected){return returned.getString() == expected.getString();}
)
{
    auto ret = func();
    test(desc,
        [&](){
            return cmpFun(ret, expected);
        }, 
        "Expected '" + expected.getString() + "' and received '" + ret.getString()+"'", 
        "Expected '" + expected.getString() + "' but received '" + ret.getString()+"'"
    );
}*/

void Tester::test(
    string desc, 
    function<TestResult()> func
)
{
    auto ret = func();
    test(desc,
        [&](){
            return ret.result;
        }, 
        "\n            "+string(testMsgPrefix)+"Expected '" + ret.expected.get<string>() + "'\n            "+string(testMsgPrefix)+"Received '" + ret.returned.get<string>()+"'", 
        "\n            "+string(testMsgPrefix)+"Expected '" + ret.expected.get<string>() + "'\n            "+string(testMsgPrefix)+"Received '" + ret.returned.get<string>()+"'" 
    );
}

void Tester::storeToFile(string fname, string content)
{
    if (fname.find("/") != string::npos)
    {
        //ensure that folder exists
        string fdir = fname.substr(0, fname.find_last_of('/'));
        system(string("mkdir -p " + fdir).c_str());
    }
                
    
    ofstream f(fname);
    f << content;
    f.close();
}

string Tester::loadFromFile(string fname, string defaultValue)
{
    ifstream a = ifstream(fname);
    string result = defaultValue;
    if (a.is_open())
    {
        a >> result;
        a.close();
    }

    return result;
}

int Tester::addMsgBusObserver(function<void(string, string, void*)> observer, string prefix)
{
    int ret = -1;
    msgBusObserversLocker.lock();
    ret = msgBusObservers.size();
    msgBusObservers.push_back(ObserverAndFilter{prefix, observer});
    msgBusObserversLocker.unlock();
    return ret;
}

void Tester::msgBusNotify(string message, string argS , void* argV)
{
    for (auto &c: msgBusObservers)
    {
        if ((c.filter == "") || (message.find(c.filter) == 0))
            c.observer(message, argS, argV);
    }
}

void Tester::delMsgObserver(int id)
{
    msgBusObservers[id] = ObserverAndFilter{"---------------------", [](string m, string a, void* v){}};
}

tuple<string, void*> Tester::msgBusWaitNext(string messagePrefix, function<void()> preAction)
{
    promise<tuple<string, void*>> prom;
    auto fut = prom.get_future();

    addMsgBusObserver([&](string msg, string payloadS, void* payloadV){
        tuple<string, void*> r;
        std::get<0>(r) = payloadS;
        std::get<1>(r) = payloadV;
        prom.set_value(r);
    }, messagePrefix);

    preAction();
    return fut.get();
}


int Tester::runTests(vector<Tester*> testers, int argc, char* argv[], string whatProjectAreBeingTested)
{
    vector<string> args;
    for (int c = 0; c < argc; c++) args.push_back(string(argv[c]));
    if (checkHelp(testers, args, whatProjectAreBeingTested))
        return 0;

    cout << "Test results:" << endl;
    //raises the contexts of the testers and gruoup them by specifcs contexts
    map<string, vector<Tester*>> contexts;// = {"all", {}};
    vector<string> contextsRunOrder;

    for (auto &c: testers)
    {
        
        auto testerContexts = c->getContexts();
        for (auto &c2: testerContexts)
        {
            if (contexts.count(c2) == 0)
            {
                contexts[c2] = {};
                contextsRunOrder.push_back(c2);
            }
            
            contexts[c2].push_back(c);
        }

        //sets ta special tag in the tester indicating that it is not runned yet. This tag will be used to prevent call more one time a tester.
        //c->setTag("main.tested", "false");
    }

    //identify required contexts (or all if no one is informed)
    //calls the testers according to the required contexts

    function<void(string context)> runContext = [&](string context){
        if (contexts.count(context) > 0)
        {
            for (auto &c: contexts[context])
                c->run(context);
        }
        else
            cerr << "        Context '" << context << "' has not found" << endl;

    };

    if (args.size() == 1 || (args.size() == 2 && args[1] == "all"))
        for (auto &c : contextsRunOrder)
        {
            cout << "    Context " << c << ":" << endl;
            runContext(c);
        }

    else
        for (size_t c =1; c < args.size(); c++)
        {
            cout << "    Context " << args[c] << ":" << endl;
            runContext(args[c]);
        }

    cout << endl << "Final statistics: "<< to_string(Tester::testsPassed + Tester::testsFailed) <<" tests performed, where " << Tester::testsPassed << " passed and " << Tester::testsFailed << " failed." << endl << flush;


    return Tester::testsFailed;
}



bool Tester::checkHelp(vector<Tester*> testers, vector<string> args, string whatProjectAreBeingTested)
{
    if (args.size() > 1 &&  args[1].find(string("-h")) != string::npos)
    {
        //shows the available context and an example of usage

        cout << "This is a test program. It is make to run tests in the code of "<< whatProjectAreBeingTested << "." << endl;
        cout << "Usage: "+args[0]+" <options>" << endl;
        cout << "options:" << endl;
        cout << "    [context]: optional context name that specifies which test context should be run. Let empty or use 'all' to run all tests." << endl;
        cout << "    -h, --help: shows this help" << endl;
        cout << "" << endl;
        cout << "Avaibale contexts are: "<< endl;
        cout << "    all (runs tests of all contexts)" << endl;

        for (auto &c: testers){
            auto testerContexts = c->getContexts();
            for (auto &c2: testerContexts)
                cout << "    " << c2 << endl;
        }
        return true;
    }

    return false;
}





#ifdef __DYNAMIC_VAR_H_
    bool Tester::assertEqualsB(DynamicVar expected, DynamicVar returned)
    {
        try{
            return expected.get<string>() == returned.get<string>();
        }   
        catch (...)
        {
            return false;
        }
    }

    bool Tester::assertDifferentB(DynamicVar expected, DynamicVar returned)
    {
        try{
            return expected.get<string>() != returned.get<string>();
        }   
        catch (...)
        {
            return false;
        }
    }

    bool Tester::assertContainsB(DynamicVar on, DynamicVar findId)
    {
        try{
            return on.get<string>().find(findId.get<string>()) != string::npos;
        }
        catch (...)
        {
            return false;
        }
    }

    bool Tester::assertNotContainsB(DynamicVar on, DynamicVar findId)
    {
        try{
            return on.get<string>().find(findId.get<string>()) == string::npos;
        }   
        catch (...)
        {
            return false;

        }
    }

    bool Tester::assertGreaterThanB(DynamicVar _it, DynamicVar thanIt)
    {
        try{
            return _it.get<double>() > thanIt.get<double>();
        }   
        catch (...)
        {
            return false;

        }
    }

    bool Tester::assertGreaterThanOrEqualsB(DynamicVar _it, DynamicVar thanIt)
    {
        try{
            return _it.get<double>() >= thanIt.get<double>();
        }   
        catch (...)
        {
            return false;

        }
    }

    bool Tester::assertLessThanB(DynamicVar _it, DynamicVar thanIt)
    {
        try{
            return _it.get<double>() < thanIt.get<double>();
        }
        catch (...)
        {
            return false;

        }
    }

    bool Tester::assertLessThanOrEqualsB(DynamicVar _it, DynamicVar thanIt)
    {
        try{
            return _it.get<double>() <= thanIt.get<double>();
        }   
        catch (...)
        {
            return false;

        }
    }

    bool Tester::assertThatB(bool expression)
    {
        try{
            return expression;
        }
        catch (...)
        {
            return false;

        }
    }

    bool Tester::assertThatB(function<bool()> expression)
    {
        try{
            return expression();
        }   
        catch (...)
        {
            return false;
        }
    }

    bool Tester::assertTrueB(bool expression)
    {
        try{
            return expression;
        }   
        catch (...)
        {
            return false;

        }
    }

    bool Tester::assertFalseB(bool expression, string message)
    {
        try{
            return !expression;
        }   
        catch (...)
        {
            return false;

        }
    }


    TestResult Tester::assertEqualsTR(DynamicVar expected, DynamicVar returned)
    {
        return TestResult{assertEqualsB(expected, returned), expected, returned};
    }

    TestResult Tester::assertDifferentTR(DynamicVar expected, DynamicVar returned)
    {
        return TestResult{assertDifferentB(expected, returned), expected, returned};
    }

    TestResult Tester::assertContainsTR(DynamicVar on, DynamicVar findId)
    {
        return TestResult{assertContainsB(on, findId), on, findId};
    }

    TestResult Tester::assertNotContainsTR(DynamicVar on, DynamicVar findId)
    {
        return TestResult{assertNotContainsB(on, findId), on, findId};
    }

    TestResult Tester::assertGreaterThanTR(DynamicVar _it, DynamicVar thanIt)
    {
        return TestResult{assertGreaterThanB(_it, thanIt), _it, thanIt};
    }

    TestResult Tester::assertGreaterThanOrEqualsTR(DynamicVar _it, DynamicVar thanIt)
    {
        return TestResult{assertGreaterThanOrEqualsB(_it, thanIt), _it, thanIt};
    }

    TestResult Tester::assertLessThanTR(DynamicVar _it, DynamicVar thanIt)
    {
        return TestResult{assertLessThanB(_it, thanIt), _it, thanIt};
    }

    TestResult Tester::assertLessThanOrEqualsTR(DynamicVar _it, DynamicVar thanIt)
    {
        return TestResult{assertLessThanOrEqualsB(_it, thanIt), _it, thanIt};
    }

    TestResult Tester::assertThatTR(bool expression, string expected, string returned)
    {
        return TestResult{assertThatB(expression), expected, returned};
    }

    TestResult Tester::assertThatTR(function<bool()> expression, string expected, string returned)
    {
        return TestResult{assertThatB(expression), expected, returned};
    }

    TestResult Tester::assertTrueTR(bool expression)
    {
        return TestResult{assertTrueB(expression), "true", "true"};
    }

    TestResult Tester::assertFalseTR(bool expression, string message)
    {
        return TestResult{assertFalseB(expression, message), "false", "false"};
    }

    void Tester::assertEquals(DynamicVar expected, DynamicVar returned, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertEqualsB(expected, returned);
        if (!ret)
            throw TestAssertException("assertEquals("+expected.getString()+", "+returned.getString() + ") failed"+additionalInfo);
    }
    void Tester::assertDifferent(DynamicVar expected, DynamicVar returned, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertDifferentB(expected, returned);
        if (!ret)
            throw TestAssertException("assertDifferent("+expected.getString()+", "+returned.getString() + ") failed"+additionalInfo);
    }
    void Tester::assertContains(DynamicVar on, DynamicVar findId, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertContainsB(on, findId);
        if (!ret)
            throw TestAssertException("assertContains("+on.getString()+", "+findId.getString() + ") failed"+additionalInfo);
    }
    void Tester::assertNotContains(DynamicVar on, DynamicVar findId, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertNotContainsB(on, findId);
        if (!ret)
            throw TestAssertException("assertNotContains("+on.getString()+", "+findId.getString() + ") failed"+additionalInfo);
    }
    void Tester::assertGreaterThan(DynamicVar _it, DynamicVar thanIt, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertGreaterThanB(_it, thanIt);
        if (!ret)
            throw TestAssertException("assertGreaterThan("+_it.getString()+", "+thanIt.getString() + ") failed"+additionalInfo);
    }
    void Tester::assertGreaterThanOrEquals(DynamicVar _it, DynamicVar thanIt, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertGreaterThanOrEqualsB(_it, thanIt);
        if (!ret)
            throw TestAssertException("assertGreaterThanOrEquals("+_it.getString()+", "+thanIt.getString() + ") failed"+additionalInfo);
    }
    void Tester::assertLessThan(DynamicVar _it, DynamicVar thanIt, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertLessThanB(_it, thanIt);
        if (!ret)
            throw TestAssertException("assertLessThan("+_it.getString()+", "+thanIt.getString() + ") failed"+additionalInfo);
    }
    void Tester::assertLessThanOrEquals(DynamicVar _it, DynamicVar thanIt, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertLessThanOrEqualsB(_it, thanIt);
        if (!ret)
            throw TestAssertException("assertLessThanOrEquals("+_it.getString()+", "+thanIt.getString() + ") failed"+additionalInfo);
    }

    void Tester::assertThat(bool expression, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertThatB(expression);
        if (!ret)
            throw TestAssertException("assertThat(bool) failed"+additionalInfo);
    }
    void Tester::assertThat(function<bool()> expression, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertThatB(expression);
        if (!ret)
            throw TestAssertException("assertThat(function<bool()>) failed"+additionalInfo);
    }
    void Tester::assertTrue(bool expression, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertTrueB(expression);
        if (!ret)
            throw TestAssertException("assertTrue("+string(expression ? "true": "false")+") failed"+additionalInfo);
    }
    void Tester::assertFalse(bool expression, string message, string additionalInfo)
    {
        if (additionalInfo != "")
            additionalInfo = ": "+additionalInfo;
        auto ret = assertFalseB(expression, message);
        if (!ret)
            throw TestAssertException("assertFalse("+string(expression ? "true": "false")+") failed"+additionalInfo);
    }

#endif
