#ifndef _API_CONTROLER_H_
    #include "APIController.h"
#endif

namespace API {
    APIController::APIController(ApiMediatorInterface *ctrl)
    {
        this->ctrl = ctrl;
        this->pPhomau = new PHOMAU(2028, this);
    }

    string APIController::GetVar(string name, string defaultValue)
    {
        return this->ctrl->GetVar(name, defaultValue);
    }

    void APIController::SetVar(string name, string value)
    {
        this->ctrl->SetVar(name, value);

        //notiry observers
        this->pPhomau->__PROTOCOL_PHOMAU_NOTIFY_OBSERVERS(name, value);
    }

    long APIController::GetVar_Int(string varname, long defaultValue)
    {
        return this->ctrl->GetVar_Int(varname, defaultValue);
    }

    void APIController::SetVar_Int(string varname, long value)
    {
        this->ctrl->SetVar_Int(varname, value);
    }

    void APIController::DelVar(string varname)
    {
        this->ctrl->DelVar(varname);
    }

    vector<string> APIController::getChildsOfVar(string parentName)
    {
        return this->ctrl->getChildsOfVar(parentName);
    }
}