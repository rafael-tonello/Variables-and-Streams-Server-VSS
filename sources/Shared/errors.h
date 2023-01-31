#ifndef _ERRORS_H_
#define _ERRORS_H_

#include <string>

class Errors{
public:
    class Error{
    private:
        static uint codeGenerationCount;
        uint code;

        uint getNewCode(){
            return Error::codeGenerationCount++;
        }
    public:
        std::string message;
        Error(uint errorCode, std::string errorMessage): code(errorCode), message(errorMessage){
            if (errorCode > Error::codeGenerationCount)
                Error::codeGenerationCount = errorCode;
        }
        Error(std::string errorMessage): message(errorMessage){
            code = getNewCode();
        }
        
        operator std::string(){return message; }
        bool operator==(const Error &err1) {return err1.code == this->code; }
        friend bool operator==(const Error &err1, const Error &err2) {return err1.code == err2.code; }

        bool operator!=(const Error &err1) {return err1.code != this->code; }
        friend bool operator!=(const Error &err1, const Error &err2) {return err1.code != err2.code; }
    };

    template<typename TResult>
    class ResultWithErrorStatus{
    public:
        TResult result;
        Error errorStatus;

        ResultWithErrorStatus(TResult result, Error errorStatus): result(result), errorStatus(errorStatus){};
        ResultWithErrorStatus(Error errorStatus, TResult result): result(result), errorStatus(errorStatus){};
    };

    static Error NoError;
    static Error Error_VariableWithWildCardCantBeSet;
    static Error Error_TheVariable_name_IsLocketAndCantBeChangedBySetVar;
    static Error Error_VariablesStartedWithUnderscornAreJustForInternal;
    static Error Error_WildCardCabBeUsedOnlyAtEndOfVarNameForVarGetting;
    static Error Error_TheVariableNameCannotBeEmpty;
};
#endif