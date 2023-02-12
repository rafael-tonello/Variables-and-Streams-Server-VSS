#ifndef __HTTP.TEST__H__ 
#define __HTTP.TEST__H__ 

#include <tester.h>
#include <string>
#include <TaggedObject.h>
#include <httpapi.h>

using namespace std;
 
class Http_test: public Tester { 
public: 
    Http_test(); 
    ~Http_test(); 

public:
    /* Tester implementation */
    vector<string> getContexts();
    void run(string context);
}; 

namespace HttpTestData
{
    class HttpStatus{
    public:
        uint code;
        string message;
    };

    class Header{
    public:
        string key;
        vector<string> values;

        string exportToHttpHeaderFormat()
        {
            string ret = key + ": ";
            for (int c = 0; c < values.size(); c++)
                ret += values[c] + (c < values.size()-1 ? "; " : "");

            return ret;
        };

        static Header createNewFromText(string headerText)
        {

        }
    };

    class Body{
    public:
        uint contentLength;
        string contentType;
        string content;

        void clear()
        {
            contentLength = 0;
            contentType = "";
            content = "";
        }
    };

    
    class HttpData: TaggedObject{
    public:
        vector<Header> headers;
        Body body;
    };

    class HttpResponse:HttpData{
    public:
        HttpStatus status;
        string resource;
    };

    class HttpRequest: HttpData{
    public:
        string url;
        string method;
    };

}
 
#endif 
