#include  "plaintextexporter.h" 

string API::HTTP::PlainTextExporter::toString()
{
    stringstream s;
    for (auto &c: vars)
    {
        s << c.first << "=" << c.second.getString() << endl;
    }
    return s.str();
}

string API::HTTP::PlainTextExporter::getMimeType()
{
    return "text/plain";
}
