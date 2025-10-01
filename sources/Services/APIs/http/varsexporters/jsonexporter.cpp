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
        string append = "";
        if (hasChildren(c.first)){
            append="._value";
            if (c.first == "")
                append = "_value";
        }
            
        result.set(c.first  + append, escape(c.second.getString()));
    }

    return result.ToJson(pretty);
}

bool API::HTTP::JsonExporter::hasChildren(string varName)
{
    for (auto &c: vars)
    {
        if (c.first.size() > varName.size() && c.first.substr(0, varName.size()+1) == varName+".")
            return true;
    }

    return false;
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