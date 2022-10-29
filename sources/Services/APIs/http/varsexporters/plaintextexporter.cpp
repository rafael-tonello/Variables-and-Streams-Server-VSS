#include  "plaintextexporter.h" 

string PlainTextExporter::toString()
{
    stringstream s;
    for (auto &c: vars)
    {
        s << c.first << "=" << c.second.getString() << endl;
    }
    return s.str();
}

string PlainTextExporter::getMimeType()
{
    return "text/plain";
}
