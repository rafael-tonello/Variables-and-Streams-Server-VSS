#ifndef __ERRORS__H__ 
#define __ERRORS__H__ 

#include <string>
#include <Utils.h>

using namespace std;
namespace Errors{
    using Error = string;

    extern Error NoError;

    extern Error Error_VariableWithWildCardCantBeSet;
    extern Error Error_TheVariable_name_IsLocketAndCantBeChangedBySetVar;
    extern Error Error_VariablesStartedWithUnderscornAreJustForInternal;
    extern Error Error_WildCardCabBeUsedOnlyAtEndOfVarNameForVarGetting;
    extern Error Error_WildcardCanotBeUsesForGetVarChilds;
    extern Error Error_TheVariableNameCannotBeEmpty;
    extern Error Error_TimeoutReached;

    Error createError(string message);
    Error createError(string message, Error nestedError);
    void forNestedErrors(Error errorWithNestedErrors, function<void(Error err)> f);

    template <typename T>
    class ResultWithStatus{
    public:
        T result;
        Error status = Errors::NoError;

        ResultWithStatus(){}
        ResultWithStatus(T result, Error error): result(result), status(error){}
        ResultWithStatus(Error error, T result): status(error), result(result){}
    };

}
#endif