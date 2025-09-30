#include  "jsonexporter.h" 

API::HTTP::JsonExporter::JsonExporter(bool pretty)
{
    this->pretty = pretty;
}

string API::HTTP::JsonExporter::toString()
{
    JSON result;
    for (auto &c: vars)
    {
        string append="._value";
        if (c.first == "")
            append = "_value";
            
        result.set(c.first  + append, escape(c.second.getString()));
    }

    return result.ToJson(pretty);
}

string API::HTTP::JsonExporter::sGetMimeType()
{
    return "application/json";
}

string API::HTTP::JsonExporter::getMimeType()
{
    return sGetMimeType();
}

bool API::HTTP::JsonExporter::checkMimeType(string mimeType)
{
    return API::HTTP::IVarsExporter::checkMimeType(mimeType, sGetMimeType());
}