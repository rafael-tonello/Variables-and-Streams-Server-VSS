#include "errors.h"

namespace Errors{
    Error NoError = "";
    Error Error_VariableWithWildCardCantBeSet = "Invalid setVar parameter. A variable with '*' can't be set";
    Error Error_TheVariable_name_IsLocketAndCantBeChangedBySetVar = "The variable 'vName' is locked and can't be changed by setVar";
    Error Error_VariablesStartedWithUnderscornAreJustForInternal = "Variabls started with underscorn (_) are just for internal flags and can't be setted by clients";
    Error Error_WildCardCabBeUsedOnlyAtEndOfVarNameForVarGetting = "Wildcard char (*) can be used only at last of varname in the getVar function";
    Error Error_WildcardCanotBeUsesForGetVarChilds = "Wildcard char (*) cannot be used for get childs of a var";
    Error Error_TheVariableNameCannotBeEmpty = "The variable name cannot be empty";
    Error Error_TimeoutReached = "Timeout reached";
}

Errors::Error Errors::createError(string message){ return message; }
Errors::Error Errors::createError(string message, Errors::Error nestedError)
{
    //return message + ":\n" + "  >" + Utils::stringReplace(nestedError, "\n", "\n  "); 
    return message + ":\n" + "  >" + Utils::stringReplace(nestedError, "\n", "\n  "); 
}

void Errors::forNestedErrors(Error errorWithNestedErrors, function<void(Error err)> f)
{
    while (true)
    {
        auto cutPos = errorWithNestedErrors.find('>');
        if( cutPos != string::npos)
        {
            auto currErr = errorWithNestedErrors.substr(0, cutPos);
            errorWithNestedErrors = errorWithNestedErrors.substr(cutPos +1);
            if (currErr != "")
                f(currErr);
        }
        else
            break;
    }

    if (errorWithNestedErrors != "")
        f(errorWithNestedErrors);
}
