#ifndef __PLAINTEXTEXPORTER__H__ 
#define __PLAINTEXTEXPORTER__H__ 

#include <ivarsexporter.h>
#include <sstream>
#include <string>

using namespace std;

namespace API::HTTP { 
    class PlainTextExporter: public IVarsExporter { 
    public: 
        string toString();
        string getMimeType();

        static string sGetMimeType();
        static bool checkMimeType(string mimeType);
    };
} 
 
#endif 
