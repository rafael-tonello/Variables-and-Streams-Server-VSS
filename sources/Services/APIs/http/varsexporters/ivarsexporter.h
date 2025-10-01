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
        /// @brief Add a variable to the exporter
        /// @param name The name of the variable
        /// @param value The value of the variable
        virtual void add(string name, DynamicVar value);
        
        /// @brief add multiple variables to the exporter (each tuple contains the name and the value of a variable)
        /// @param varsToAdd a vector of tuples, where each tuple contains the name and the value of a variable
        virtual void add(vector<tuple<string, DynamicVar>> varsToAdd);

        /// @brief Clear all added variables
        virtual void clear();

        /// @brief serialize all added variables to a string, in the format of the exporter
        /// @return The serialized string
        virtual operator string(){return toString(); }

        /// @brief serialize all added variables to a string, in the format of the exporter
        virtual operator const char*(){return string().c_str(); }

        static bool checkMimeType(string mimeType1, string mimeType2);
    public:

        /// @brief Serialize all added variables to a string, in the format of the exporter
        /// @return The serialized string
        virtual string toString() = 0;

        /// @brief Get the mime type of the format of the exporter
        /// @return The mime type as a string
        virtual string getMimeType() = 0;
    }; 
}
 
#endif 
