#ifndef __HTTP_TEST__H__ 
#define __HTTP_TEST__H__ 

#include <tester.h>
#include <string>
#include <TaggedObject.h>
#include <httpapi.h>
#include <ApiMediatorInterface.h>
#include <dependencyInjectionManager.h>
#include <utils.h>
#include <apimediatorinterfacemock.h>
#include <iloggermock.h>
#include <InMemoryConfProvider.h>

using namespace std;

class HttpStatus{
public:
    uint code;
    string message;
};

class Header{
public:
    string key;
    vector<string> values;

    string getValuesAsString()
    {
        string ret = 0;
        for (auto &c: values)
            ret += c +"; ";

        ret = ret.substr(0, ret.size()-2);

        return ret;
    }

    string exportToHttpHeaderFormat()
    {
        string ret = key + ": ";
        for (int c = 0; c < values.size(); c++)
            ret += values[c] + (c < values.size()-1 ? "; " : "");

        return ret;
    };

    static Header createFromHeaderText(string headerText)
    {
        //todo: implement
        return Header();
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


class HttpData: public TaggedObject{
public:
    vector<Header> headers;
    Body body;
};

class HttpResponse: public HttpData{
public:
    HttpStatus status;
    string resource;
};

class HttpRequest: public HttpData{
public:
    string url;
    string method;
};

class Http_test: public Tester { 
public: 
    string version = "instance for tests";

    Http_test();
    ~Http_test();
    HttpResponse request(HttpRequest request);
    string readSocket(int socket);
    void writeSocket(int socket, string data);
    void doRequest(HttpRequest request, int socket);
    void processReadByte(string byte, HttpResponse& out);
    HttpResponse readResponse(int socket);

public:
    /* Tester implementation */
    vector<string> getContexts();
    void run(string context);
}; 

 
#endif 
