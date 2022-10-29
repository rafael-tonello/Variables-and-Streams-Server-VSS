#include  "jsonexporter.h" 
 

string JsonExporter::toString()
{
    JSON result;
    for (auto &c: vars)
        result.setString(c.first, c.second, true);

    return result.ToJson();
}

string JsonExporter::getMimeType()
{
    return "application/json";
}
