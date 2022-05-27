#ifndef __DYNAMIC_VAR_H_
#define __DYNAMIC_VAR_H_
#define INVALID_INT 0b11111111111111111111111111111111;

#include <functional>
#include <string>
#include <algorithm>

namespace Shared{
    using namespace std;
    class DynamicVar{
        string __data;
        public:
            DynamicVar();
            DynamicVar(char* value);
            DynamicVar(string value);
            DynamicVar(int value);
            DynamicVar(double value);
            DynamicVar(bool value);
            DynamicVar(int64_t value);

            int getInt(function<void()> onError = [](){});
            void setInt(int value);

            int64_t getInt64(function<void()> onError = [](){});
            void setInt64(int64_t value);

            double getDouble(function<void()> onError = [](){});
            void setDouble(double value);

            string getString();
            void setString(string value);
            void setCStr(char* s);

            bool getBool(function<void()> onError = [](){});
            void setBool(bool value);

            bool isEquals(DynamicVar* other);
            bool isEquals(DynamicVar other);
    };
}
#endif