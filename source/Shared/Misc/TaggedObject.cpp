#include "TaggedObject.h"
void TaggedObject::setTag(string tag, string value)
{
    tags[tag] = value;
}

string TaggedObject::getTag(string tag, string defaultValue)
{
    if (tags.count(tag) > 0)
        return tags[tag];
    else 
        return defaultValue;
}
