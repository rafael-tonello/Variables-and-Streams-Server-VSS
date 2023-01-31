#include "errors.h"
uint Errors::Error::codeGenerationCount = 0;

Errors::Error Errors::NoError = Errors::Error("");
Errors::Error Errors::Error_VariableWithWildCardCantBeSet = Errors::Error("Invalid setVar parameter. A variable with '*' can't be set");
Errors::Error Errors::Error_TheVariable_name_IsLocketAndCantBeChangedBySetVar = Errors::Error("The variable 'vName' is locked and can't be changed by setVar");
Errors::Error Errors::Error_VariablesStartedWithUnderscornAreJustForInternal = Errors::Error("Variabls started with underscorn (_) are just for internal flags and can't be setted by clients");
Errors::Error Errors::Error_WildCardCabBeUsedOnlyAtEndOfVarNameForVarGetting = Errors::Error("wildcard char (*) can be used only at last of varname in the getVar function");
Errors::Error Errors::Error_TheVariableNameCannotBeEmpty = Errors::Error("the variable name cannot be empty");
