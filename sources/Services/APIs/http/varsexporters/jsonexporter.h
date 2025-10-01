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
    private:
        bool pretty;

        //check if a variable has children
        
        
        /// @brief Check if a variable has children
        //for example, if we have the variables:
        // - test.var1
        // - test.var2
        // - test2.var1
        // - test2.var2.var3
        // - test2.var2.var4
        // - test3
        //  we have that test and test2 has children, but test3, var1, var2, var3 and var4 has not,
        /// @param varName The name of the variable
        /// @return True if the variable has children, false otherwise
        bool hasChildren(string varName);
    public: 
        /* IVarsExporter interface */

        /// @brief creates a new JSON exporter.
        /// @param pretty If true, the JSON will be pretty printed (with indentation and line breaks), otherwise it will be in a single line
        JsonExporter(bool pretty = false);
        
        /// @brief serialize all added variables to a JSON string
        /// @return The serialized string
        string toString();

        /// @brief Get the mime type of the format of the exporter
        /// @return The mime type as a string
        string getMimeType();

        /// @brief Get the mime type of the format of the exporter
        /// @return The mime type as a string
        static string sGetMimeType();

        /// @brief Check if the given mime type is compatible with JSON exporter
        /// @param mimeType The mime type to check
        /// @return True if the mime type is compatible with JSON exporter, false otherwise
        static bool checkMimeType(string mimeType);

    }; 
}
 
#endif 
