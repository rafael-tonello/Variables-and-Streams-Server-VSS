#include "SimpleConfFileProvider.test.h"

vector<string> SimpleConfFileProviderTester::getContexts()
{
    return {"Confs.SimpleConfFileProvider"};

}

void SimpleConfFileProviderTester::run(string context)
{
    if (context == "Confs.SimpleConfFileProvider")
    {

        //create a temporary file with key and values
        string configData = 
            string("key1 = value1\t  \n")+
            string("key2= value2 \n")+
            string("\tkey3 =value3     \n")+
            string("  key4:value4\n")+
            string("      key5 :value5\n")+
            string("key6-value6\n")+
            string("\n")+
            string("#acomment=withconfiguratinstructure\n")+
            string("//another comment\n")+
            string(";this is also a comment\n")+
            string("an_invalid_configuration invalid_value\n")+
            string("this is an invalid configuration, must be ignored\n")
        ;

        ofstream outFile("/tmp/SimpleConfFileProvider_fileForTests.tmp", ios::out);
        outFile << configData;
        outFile.close();

        //create a new SimpleConfFileProvider that reads the temporary file
        SimpleConfFileProvider confProvider("/tmp/SimpleConfFileProvider_fileForTests.tmp");

        this->test("SimpleConfFileProvider::ltrim should remove invalid chars (test 1)", [&](){
            return confProvider.ltrim(" test") == "test";
        });

        this->test("SimpleConfFileProvider::ltrim should remove invalid chars (test 2)", [&](){
            return confProvider.ltrim("\ttest") == "test";
        });

        this->test("SimpleConfFileProvider::ltrim should remove invalid chars (test 3)", [&](){
            return confProvider.ltrim("\t \t\t   \t \ttest\t ") == "test\t ";
        });

        this->test("SimpleConfFileProvider::rtrim should remove invalid chars (test 1)", [&](){
            return confProvider.rtrim("test ") == "test";
        });

        this->test("SimpleConfFileProvider::rtrim should remove invalid chars (test 2)", [&](){
            return confProvider.rtrim("test\t") == "test";
        });

        this->test("SimpleConfFileProvider::rtrim should remove invalid chars (test 3)", [&](){
            return confProvider.rtrim(" \ttest\t \t\t   \t \t") == " \ttest";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should detect the separator (test 1)", [&](){
            return confProvider.identifyKeyValueSeparator("key=value")  == "=";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should detect the separator (test 2)", [&](){
            return confProvider.identifyKeyValueSeparator("key:value")  == ":";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should not detect a valid separator", [&](){
            return confProvider.identifyKeyValueSeparator("key-value")  != "-";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should not detect a valid separator", [&](){
            return confProvider.identifyKeyValueSeparator("keyvalue")  == "";
        });


        //run the tests
        this->test("Only 5 configurations should be loaded", [&]
        {
            auto configs = confProvider.readAllConfigurations();

            return TestResult{
                .result = configs.size() == 5,
                .expected = "5",
                .returned = to_string(configs.size())
            };
        });

        this->test("Configurations names should have 4 chars of size", [&]
        {
            auto configs = confProvider.readAllConfigurations();
            string expected = "key1: 4, key2: 4, key3: 4, key4: 4, key5: 4";
            string result = "";
            for (tuple<string, string> curr: configs)
                result += get<0>(curr) + ": "+to_string(get<0>(curr).size())+", ";

            result = result != "" ? result.substr(0, result.size() -2) : "";

            return TestResult{
                .result = expected == result,
                .expected = expected,
                .returned = result

            };
        });

        this->test("Configurations values should have 6 chars of size", [&]
        {
            auto configs = confProvider.readAllConfigurations();
            string expected = "value1: 6, value2: 6, value3: 6, value4: 6, value5: 6";
            string result = "";
            for (tuple<string, string> curr: configs)
                result += get<1>(curr) + ": "+to_string(get<1>(curr).size())+", ";

            result = result != "" ? result.substr(0, result.size() -2) : "";

            return TestResult{
                .result = expected == result,
                .expected = expected,
                .returned = result

            };
        });
    }
}
