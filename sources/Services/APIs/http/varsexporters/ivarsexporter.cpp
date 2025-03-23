#include "ivarsexporter.h"

string API::HTTP::IVarsExporter::escape(string source)
{
    string ret = "";
    for (auto &c: source)
    {
        if (c == '\n')
            ret += "\\n";
        else if (c == '\r')
            ret += "\\r";
        else if (c == '\t')
            ret += "\\t";
        else if (c == '\\')
            ret += "\\\\";
        else if (c == '\"')
            ret += "\\\"";
        else
            ret += c;
    }
    return ret;
}


void API::HTTP::IVarsExporter::add(string name, DynamicVar value)
{
    this->vars[name] = value;
}

void API::HTTP::IVarsExporter::add(vector<tuple<string, DynamicVar>> varsToAdd)
{
    for (auto &c: varsToAdd)
        this->add(std::get<0>(c), std::get<1>(c));
}

void API::HTTP::IVarsExporter::clear()
{
    this->vars.clear();
}

bool API::HTTP::IVarsExporter::checkMimeType(string mimeType1, string mimeType2)
{
    std::transform(mimeType1.begin(), mimeType1.end(), mimeType1.begin(), [](unsigned char c){ return std::tolower(c); });
    std::transform(mimeType2.begin(), mimeType2.end(), mimeType2.begin(), [](unsigned char c){ return std::tolower(c); });

    return mimeType1.find(mimeType2) != string::npos || mimeType2.find(mimeType1) != string::npos;
}

