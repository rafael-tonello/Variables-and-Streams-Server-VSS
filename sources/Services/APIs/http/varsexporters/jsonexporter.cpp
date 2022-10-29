#include  "jsonexporter.h" 
 

string API::HTTP::JsonExporter::toString()
{
    JSON result;
    for (auto &c: vars)
        result.setString(c.first, c.second, true);

    return result.ToJson();
}

string API::HTTP::JsonExporter::getMimeType()
{
    return "application/json";
}
