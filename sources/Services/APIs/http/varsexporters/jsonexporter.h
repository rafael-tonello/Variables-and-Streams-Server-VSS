#ifndef __JSONEXPORTER__H__ 
#define __JSONEXPORTER__H__ 

#include <string>
#include <vector>
#include <tuple>
#include <ivarsexporter.h>
#include <DynamicVar.h>
#include <JSON.h>


using namespace std;
using namespace JsonMaker;
namespace API::HTTP {
    class JsonExporter: public IVarsExporter { 
    public: 
        string toString();
        string getMimeType();

        static string sGetMimeType();
        static bool checkMimeType(string mimeType);

    }; 
}
 
#endif 
