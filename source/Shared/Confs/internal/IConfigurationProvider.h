#ifndef _CONF_ICONFIGURATION_PROVIDER_H_
#define _CONF_ICONFIGURATION_PROVIDER_H_
#include <vector>
#include <tuple>
#include <string>
#include <functional>

using namespace std;
namespace Shared{
    using IConfigurationProvider_onData = function<void (vector<tuple<string, string>> configurations)>;
    class IConfigurationProvider
    {
    public:
        virtual void readAndObservate(IConfigurationProvider_onData onData) = 0;
    };
}

#endif