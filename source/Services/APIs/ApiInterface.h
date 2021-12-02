#ifndef _APIMEDIATORINTERFACE_H_
#define _APIMEDIATORINTERFACE_H_
#include <iostream>
#include <string>
#include <vector>
#include <future>
#include "../../Shared/Misc/DynamicVar.h"

using namespace Shared;
using namespace std;
namespace API
{

    using namespace Shared;
    
    class ApiInterface
    {
    public:
        virtual string getApiId() = 0;

    };
};
#endif
