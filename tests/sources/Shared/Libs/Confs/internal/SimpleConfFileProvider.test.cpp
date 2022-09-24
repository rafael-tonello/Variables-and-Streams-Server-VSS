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

        writeFile(
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
        );

        //create a new SimpleConfFileProvider that reads the temporary file
        SimpleConfFileProvider *confProvider = new SimpleConfFileProvider(defaultFile);

        this->test("SimpleConfFileProvider::ltrim should remove invalid chars (test 1)", [&](){
            return confProvider->ltrim(" test") == "test";
        });

        this->test("SimpleConfFileProvider::ltrim should remove invalid chars (test 2)", [&](){
            return confProvider->ltrim("\ttest") == "test";
        });

        this->test("SimpleConfFileProvider::ltrim should remove invalid chars (test 3)", [&](){
            return confProvider->ltrim("\t \t\t   \t \ttest\t ") == "test\t ";
        });

        this->test("SimpleConfFileProvider::rtrim should remove invalid chars (test 1)", [&](){
            return confProvider->rtrim("test ") == "test";
        });

        this->test("SimpleConfFileProvider::rtrim should remove invalid chars (test 2)", [&](){
            return confProvider->rtrim("test\t") == "test";
        });

        this->test("SimpleConfFileProvider::rtrim should remove invalid chars (test 3)", [&](){
            return confProvider->rtrim(" \ttest\t \t\t   \t \t") == " \ttest";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should detect the separator (test 1)", [&](){
            return confProvider->identifyKeyValueSeparator("key=value")  == "=";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should detect the separator (test 2)", [&](){
            return confProvider->identifyKeyValueSeparator("key:value")  == ":";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should not detect a valid separator", [&](){
            return confProvider->identifyKeyValueSeparator("key-value")  != "-";
        });

        this->test("SimpleConfFileProvider::identifyKeyValueSeparator should not detect a valid separator", [&](){
            return confProvider->identifyKeyValueSeparator("keyvalue")  == "";
        });


        //run the tests
        this->test("Only 5 configurations should be loaded", [&]
        {
            auto configs = confProvider->readAllConfigurations();

            return TestResult{
                configs.size() == 5,
                "5",
                to_string(configs.size())
            };
        });

        this->test("Configurations names should have 4 chars of size", [&]
        {
            auto configs = confProvider->readAllConfigurations();
            string expected = "key1: 4, key2: 4, key3: 4, key4: 4, key5: 4";
            string result = "";
            for (tuple<string, string> curr: configs)
                result += get<0>(curr) + ": "+to_string(get<0>(curr).size())+", ";

            result = result != "" ? result.substr(0, result.size() -2) : "";

            return TestResult{
                expected == result,
                expected,
                result

            };
        });

        this->test("Configurations values should have 6 chars of size", [&]
        {
            auto configs = confProvider->readAllConfigurations();
            string expected = "value1: 6, value2: 6, value3: 6, value4: 6, value5: 6";
            string result = "";
            for (tuple<string, string> curr: configs)
                result += get<1>(curr) + ": "+to_string(get<1>(curr).size())+", ";

            result = result != "" ? result.substr(0, result.size() -2) : "";

            return TestResult{
                expected == result,
                expected,
                result

            };
        });

        delete confProvider;

        writeFile(
            string("key1=value1\n")+
            string("key2=value2\n")
        );

        confProvider = new SimpleConfFileProvider(defaultFile);

        string conf1 = "";
        string conf2 = "";

        //observate changes in the file
        confProvider->readAndObservate([&](vector<tuple<string, string>> configurations)
        {
            for (auto &c: configurations)
            {
                //cout << get<0>(c) << " -> " << get<1>(c) << endl;
                if (get<0>(c) == "key1")
                    conf1 = get<1>(c);
                else if (get<0>(c) == "key2")
                    conf2 = get<1>(c);
            }
        });

        //at this point, the key1 and key2 variables should have the values "value1" and "value 2"
        this->test("conf1 and conf2 should have the values \"value1\" and \"value2\"", [&]
        {
            string expected = "value1, value2";
            string received = conf1 + ", " + conf2;
            return TestResult
            {
                received == expected,
                expected,
                received
            };
        });

        //at this point, the key1 and key2 variables should have the values "value1" and "value 2"
        this->test("conf1 and conf2 should have the values \"value1\" and \"value2_changed\"", [&]
        {
            //change the values
            writeFile(
                string("key1=value1\n")+
                string("key2=value2_changed\n")
            ); 
            
            string expected = "value1, value2_changed";
            string received = conf1 + ", " + conf2;
            return TestResult
            {
                received == expected,
                expected,
                received
            };
        });

        //at this point, the key1 and key2 variables should have the values "value1" and "value 2"
        this->test("conf1 and conf2 should have the values \"value1_changed\" and \"value2\"", [&]
        {
            //change the values
            writeFile(
                string("key1=value1_changed\n")+
                string("key2=value2\n")
            ); 
            
            string expected = "value1_changed, value2";
            string received = conf1 + ", " + conf2;
            return TestResult
            {
                received == expected,
                expected,
                received
            };
        });

        //at this point, the key1 and key2 variables should have the values "value1" and "value 2"
        this->test("conf1 and conf2 should have the values \"value1_changed\" and \"value2_changed\"", [&]
        {
            //change the values
            writeFile(
                string("key1=value1_changed\n")+
                string("key2=value2_changed\n")
            ); 
            
            string expected = "value1_changed, value2_changed";
            string received = conf1 + ", " + conf2;
            return TestResult
            {
                received == expected,
                expected,
                received
            };
        });
    }
}

void SimpleConfFileProviderTester::writeFile(string data)
{
    this->writeFile(defaultFile, data);
}

void SimpleConfFileProviderTester::writeFile(string fName, string data)
{
    ofstream outFile(fName, ios::out);
        outFile << data;
        outFile.close();
    
    usleep(500000);
}


