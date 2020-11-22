#ifndef _API_CONTROLER_H_
#define _API_CONTROLER_H_

#include "ApiMediatorInterface.h"
#include "PHOMAU/PHOMAU.h"
#include <vector>
namespace API {
    class APIController: public ApiMediatorInterface{
        private:
            ApiMediatorInterface *ctrl;
            PHOMAU *pPhomau;
        public:
            APIController(ApiMediatorInterface *ctrl);
            string GetVar(string name, string defaultValue);
            void SetVar(string name, string value);
            long GetVar_Int(string varname, long defaultValue);
            void SetVar_Int(string varname, long value);
            void DelVar(string varname);
            vector<string> getChildsOfVar(string parentName);
    };
}

#endif