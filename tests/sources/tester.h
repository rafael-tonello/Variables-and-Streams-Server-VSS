#ifndef __TESTER_H
#define __TESTER_H


#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <regex>
#include <fstream>
#include <mutex>
#include <future>
#include <DynamicVar.h>

using namespace std;
struct TestResult{
    bool result;
    DynamicVar expected;
    DynamicVar returned;
};

struct ObserverAndFilter{
    string filter;
    function<void(string, string, void*)> observer;
};

//inherits std::exception
class TestAssertException: public exception{
    string message;
public:
    TestAssertException(string message): message(message){}
    const char* what() const throw(){
        return message.c_str();
    }
};

class Tester{
private:
    string testMsgPrefix = "";
    static bool checkHelp(vector<Tester*> testers, vector<string> args, string whatProjectAreBeingTested);
public: /* interface to be implemented by child classes */
    virtual vector<string> getContexts() = 0;
    virtual void run(string context) = 0;
public: //static
    //messagebus system (for more complex responses and behavior analysis)
    static mutex msgBusObserversLocker;
    static vector<ObserverAndFilter> msgBusObservers;
    static int addMsgBusObserver(function<void(string, string, void*)> observer, string prefix = "");
    static void msgBusNotify(string message, string argS = "", void* argV = NULL);
    static void delMsgObserver(int id);
    //this function waits for next message with message prefix (messagePrefix), preAction can be used to 
    //grant a execution of code imediatelly after add the observer and prevent loses of any message.
    static tuple<string, void*> msgBusWaitNext(string messagePrefix, function<void()> preAction = [](){});

    //globalTest result (for simple replicating responses)
    static string global_test_result;

    //these two properties is used to mains a counting of passed and failed tests
    static uint testsFailed;
    static uint testsPassed;

    //utils function to storage and read files 
    static void storeToFile(string fname, string content);
    static string loadFromFile(string fname, string defaultValue = "");

public:
    

    map<string, string> tags;
    void setTag(string tag, string value);
    string getTag(string tag, string defValue = "");


    void errorMessage(string message);

    void redMessage(string message);
    void greenMessage(string message);
    void yellowMessage(string message);
    void blueMessage(string message);
    void cyanMessage(string message);
    void brightMessage(string message);

    void disableStdout();
    void enableStdout();

    void setTestsMessagesPrefix(string prefix);

    void test(string desc, function<bool()> func, string passMessage = "", string failMessage = "");



    #ifdef __DYNAMIC_VAR_H_
        //B comes from 'B'oolean
        //return false if the assert can't be done
        static bool assertEqualsB(DynamicVar expected, DynamicVar returned);
        //return false if the assert can't be done
        static bool assertDifferentB(DynamicVar expected, DynamicVar returned);
        //return false if the assert can't be done
        static bool assertContainsB(DynamicVar on, DynamicVar findId);
        //return false if the assert can't be done
        static bool assertNotContainsB(DynamicVar on, DynamicVar findId);
        //return false if the assert can't be done
        static bool assertGreaterThanB(DynamicVar _it, DynamicVar thanIt);
        //return false if the assert can't be done
        static bool assertGreaterThanOrEqualsB(DynamicVar _it, DynamicVar thanIt);
        //return false if the assert can't be done
        static bool assertLessThanB(DynamicVar _it, DynamicVar thanIt);
        //return false if the assert can't be done
        static bool assertLessThanOrEqualsB(DynamicVar _it, DynamicVar thanIt);
        //return false if the assert can't be done
        static bool assertThatB(bool expression);
        //return false if the assert can't be done
        static bool assertThatB(function<bool()> expression);
        //return false if the assert can't be done
        static bool assertTrueB(bool expression);
        //return false if the assert can't be done
        static bool assertFalseB(bool expression, string message);

        //TR comes from 'T'est'R'esult
        //returns a TestResult object.
        static TestResult assertEqualsTR(DynamicVar expected, DynamicVar returned);
        //returns a TestResult object.
        static TestResult assertDifferentTR(DynamicVar expected, DynamicVar returned);
        //returns a TestResult object.
        static TestResult assertContainsTR(DynamicVar on, DynamicVar findId);
        //returns a TestResult object.
        static TestResult assertNotContainsTR(DynamicVar on, DynamicVar findId);
        //returns a TestResult object.
        static TestResult assertGreaterThanTR(DynamicVar _it, DynamicVar thanIt);
        //returns a TestResult object.
        static TestResult assertGreaterThanOrEqualsTR(DynamicVar _it, DynamicVar thanIt);
        //returns a TestResult object.
        static TestResult assertLessThanTR(DynamicVar _it, DynamicVar thanIt);
        //returns a TestResult object.
        static TestResult assertLessThanOrEqualsTR(DynamicVar _it, DynamicVar thanIt);
        //returns a TestResult object.
        static TestResult assertThatTR(bool expression, string expected, string returned);
        //returns a TestResult object.
        static TestResult assertThatTR(function<bool()> expression, string expected, string returned);
        //returns a TestResult object.
        static TestResult assertTrueTR(bool expression);
        //returns a TestResult object.
        static TestResult assertFalseTR(bool expression, string message);

        
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertEquals(DynamicVar expected, DynamicVar returned, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertDifferent(DynamicVar expected, DynamicVar returned, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertContains(DynamicVar on, DynamicVar findId, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertNotContains(DynamicVar on, DynamicVar findId, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertGreaterThan(DynamicVar _it, DynamicVar thanIt, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertGreaterThanOrEquals(DynamicVar _it, DynamicVar thanIt, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertLessThan(DynamicVar _it, DynamicVar thanIt, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertLessThanOrEquals(DynamicVar _it, DynamicVar thanIt, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertThat(bool expression, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertThat(function<bool()> expression, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertTrue(bool expression, string additionalInfo = "");
        //raise an exception if assert can't be done. 'additionalInfo' will be added to the exception message
        static void assertFalse(bool expression, string message, string additionalInfo = "");


    #endif


    

    template <class T>
    void test(
        string desc, 
        function<T()> func, 
        T expected, 
        function<bool(T, T)> compareFunc = [](T c1, T c2)
        { 
            return c1 == c2;
        },  
        string passMessage = "", 
        string failMessage = ""
    );

    static int runTests(vector<Tester*> testers, int argc = 0, char* argv[] = NULL, string whatProjectAreBeingTested = "another program");

    



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

    void test(
        string desc, 
        function<TestResult()> func
    );


};

/*
    Examples:
int testefunc(int v1, int v2)
{
    //return 2;
    return v1 + v2;
}


//obs: GuiTester implementes the class Tester
void GuiTester::run(string context)
{
    this->test("testeFunc(1, 2) must return 3", [](){
        return Shared::DynamicVar(testefunc(1, 2));
    }, Shared::DynamicVar(3));

    this->test("testeFunc(1, 1) must return 2", [](){
        return testefunc(1, 1) == 2;
    });

    this->test("testeFunc(1, 5) must return >= 6", [](){
        auto res = testefunc(1, 5);
        return TestResult {
            res >= 6,
            ">=6",
            to_string(res)
        };
    });
    
    this->test("testeFunc(1, 5) must return > 5", [](){
        auto res = testefunc(1, 5);
        return TestResult {
            res >= 5,
            ">=5",
            to_string(res)
        };
    });
}
*/

#endif