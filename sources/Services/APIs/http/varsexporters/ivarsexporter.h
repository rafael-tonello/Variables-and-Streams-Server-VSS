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
    public: 
        virtual void add(string name, DynamicVar value)
        {
            this->vars[name] = value;
        };
        virtual void add(vector<tuple<string, DynamicVar>> varsToAdd)
        {
            for (auto &c: varsToAdd)
                this->add(std::get<0>(c), std::get<1>(c));
        }
        virtual void clear()
        {
            this->vars.clear();
        }

        virtual operator string(){return toString(); }
        virtual operator const char*(){return toString().c_str(); }

        static bool checkMimeType(string mimeType1, string mimeType2)
        {
            std::transform(mimeType1.begin(), mimeType1.end(), mimeType1.begin(), [](unsigned char c){ return std::tolower(c); });
            std::transform(mimeType2.begin(), mimeType2.end(), mimeType2.begin(), [](unsigned char c){ return std::tolower(c); });

            return mimeType1.find(mimeType2) != string::npos || mimeType2.find(mimeType1) != string::npos;
        }
    public:
        virtual string toString() = 0;
        virtual string getMimeType() = 0;
    }; 
}
 
#endif 
