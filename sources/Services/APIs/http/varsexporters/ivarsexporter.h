#ifndef __IVAREXPORTER__H__ 
#define __IVAREXPORTER__H__ 

#include <string>
#include <vector>
#include <tuple>
#include <DynamicVar.h>
#include <map>

using namespace std;


using namespace std;
namespace API::HTTP{ 
    class IVarsExporter { 
    protected:
        map<string, DynamicVar> vars;

        virtual string escape(string source);
    public: 
        virtual void add(string name, DynamicVar value);
        virtual void add(vector<tuple<string, DynamicVar>> varsToAdd);
        virtual void clear();

        virtual operator string(){return string(); }
        virtual operator const char*(){return string().c_str(); }

        static bool checkMimeType(string mimeType1, string mimeType2);
    public:
        virtual string toString() = 0;
        virtual string getMimeType() = 0;
    }; 
}
 
#endif 
