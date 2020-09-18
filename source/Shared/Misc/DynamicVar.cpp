#include "DynamicVar.h"

namespace Shared{
    DynamicVar::DynamicVar()
    {
        __data = "";
    }

    DynamicVar::DynamicVar(string value)
    {
        setString(value);
    }

    DynamicVar::DynamicVar(int value)
    {
        setInt(value);
    }

    DynamicVar::DynamicVar(double value)
    {
        setDouble(value);
    }

    DynamicVar::DynamicVar(bool value)
    {
        setBool(value);
    }


    int DynamicVar::getInt(function<void()> onError)
    {
        int ret = 0;
        try
        {
            ret = std::stoi(__data, nullptr);
        }
        catch(...)
        {
            onError();
        }
        return ret;
    }

    void DynamicVar::setInt(int value)
    {
        __data = std::to_string(value);
    }


    double DynamicVar::getDouble(function<void()> onError)
    {
        double ret = 0.0;
        try
        {
            ret = std::stof(__data, nullptr);
        }
        catch(...)
        {
            onError();
        }
        return ret;
    }

    void DynamicVar::setDouble(double value)
    {
        __data = std::to_string(value);
    }


    string DynamicVar::getString()
    {
        return __data;
    }

    void DynamicVar::setString(string value)
    {
        __data = value;
    }


    bool DynamicVar::getBool(function<void()> onError)
    {
        bool ret = false;
        try
        {
            string temp = __data;
            std::transform(temp.begin(),
                        temp.end(),
                        temp.begin(),
                        ::tolower);

            ret = temp.find("trueyes1ok") != string::npos;
        }
        catch(...){
            onError();
        }
        return ret;
    }

    void DynamicVar::setBool(bool value)
    {
        if (value)
            __data = "1";
        else
            __data = "0";
    }
}