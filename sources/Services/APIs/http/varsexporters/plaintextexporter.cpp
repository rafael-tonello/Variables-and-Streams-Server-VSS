#include  "plaintextexporter.h" 

string API::HTTP::PlainTextExporter::toString()
{
    stringstream s;
    for (auto &c: vars)
    {
        if (c.first == "")
            s << c.second.getString() << endl;
        else
            s << c.first << "=" << escape(c.second.getString()) << endl;
    }
    return s.str();
}

string API::HTTP::PlainTextExporter::sGetMimeType()
{
    return "text/plain";
}

string API::HTTP::PlainTextExporter::getMimeType()
{
    return sGetMimeType();
}

bool API::HTTP::PlainTextExporter::checkMimeType(string mimeType)
{
    return API::HTTP::IVarsExporter::checkMimeType(mimeType, sGetMimeType());
}