#ifndef _CONF_ICONFIGURATION_PROVIDER_H_
#define _CONF_ICONFIGURATION_PROVIDER_H_
#include <vector>
#include <tuple>
#include <string>

using namespace std;
namespace Shared{
    class IConfigurationProvider
    {
    public:
        virtual vector<tuple<string, string>> readAllConfigurations() = 0;
    };
}

#endif