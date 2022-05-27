#ifndef __TAGGEDOBJECT
#define __TAGGEDOBJECT

#include<string>
#include<map>

using namespace std;
class TaggedObject
{
public:
    map<string, string> tags;
    void setTag(string tag, string value);
    string getTag(string tag, string defaultValue = "");

};

#endif