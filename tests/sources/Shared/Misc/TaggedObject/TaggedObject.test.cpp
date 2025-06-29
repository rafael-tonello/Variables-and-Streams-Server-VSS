#include  "TaggedObject.test.h" 
 
TaggedObjectTester::TaggedObjectTester() 
{ 
     
} 
 
TaggedObjectTester::~TaggedObjectTester() 
{ 
     
}

 
vector<string> TaggedObjectTester::getContexts()
{
    return { "TaggedObject" };
}

void TaggedObjectTester::run(string context)
{
    if (context != "TaggedObject")
        return;

    this->test("Tag should be created", [&](){
        TaggedObjectTesterTempClass t;
        t.setTag("testTag", "value");
        
        return t.tags.count("testTag");
    });

    this->test("Should get correctly a tag", [&](){
        TaggedObjectTesterTempClass t2;
        
        t2.tags["testTag2"] = "value 2";

        return t2.getTag("testTag2", "incorrect value") == "value 2";


    });

    this->test("Should return the defualt value if tag doesn't exists", [&](){
        TaggedObjectTesterTempClass t3;

        return t3.getTag("testTag3", "defaultValue") == "defaultValue";
    });

    this->test("Set on an existing tag should change its value", [&](){
        TaggedObjectTesterTempClass t4;

        t4.setTag("a", "a");
        t4.setTag("a", "b");

        return t4.getTag("a", "a") == "b";
    });

    this->test("If tags map is cleared, the getTag should return the defaultValue", [&](){
        TaggedObjectTesterTempClass t5;

        t5.setTag("a", "a");

        t5.tags.clear();

        return t5.getTag("a", "defaultValue") == "defaultValue";
    });
}
