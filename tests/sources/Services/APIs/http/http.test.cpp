#include  "http.test.h" 
 
Http_test::Http_test() 
{ 
     
} 
 
Http_test::~Http_test() 
{ 
     
} 
 
/* Tester implementation */
vector<string> Http_test::getContexts()
{
    return { string("APIs.Http") };
}


void Http_test::run(string context)
{
    if (context != string("APIs.Http"))
        return;

    DependencyInjectionManager dim;

    dim.addSingleton<string>(&version, {"version", "systemVersion", "infoVersion", "INFO_VERSION", "SYSTEM_VERSION"});
    dim.addSingleton<ILogger>(new ILoggerMock(true));
    dim.addSingleton<MessageBus<JsonMaker::JSON>>(new MessageBus<JsonMaker::JSON>([](JsonMaker::JSON item){
        return item.getChildsNames("").size() == 0;
    }));
    dim.addSingleton<Confs>(new Confs(
        vector<IConfProvider*>{
            new InMemoryConfProvider(
                vector<tuple<string, string>>{
                    { "httpApiReturnsFullPaths", "false" }
                }
            )
        }
    ));

    dim.addSingleton<ApiMediatorInterface>(new ApiMediatorInterfaceMock());
    API::HTTP::HttpAPI api(40000, 40001, &dim);
    
    test("Should return 204 when setting a var", [](){
        auto result=Utils::ssystem("curl -sS -X POST -d \"test\"  \"http://localhost:40000/n0/testvar\" -o - -w \"%{http_code}\"");
        Tester::assertEquals(204, result.output);

        return true;
    });

    //((ApiMediatorInterfaceMock*)(dim.get<ApiMediatorInterface>()))->setOnGetVar([](string name, DynamicVar defaultValue) -> future<GetVarResult>{})
    
    //TODO: Http api should return valid 'get requests' with a 200 status
    //TODO: Http api should return a 201 no content when setting a var
    //TODO: HTTP API shluld return a forbiden code (401) when try to setting flags and '*' vars
    //TODO: HTTP api should return a fobiden code (401) when try to get a var without sending its name

}


HttpResponse Http_test::request(HttpRequest request)
{
    //int socket = createSocket();

    //doRequest(request, socket);
    //return readResponse(socket);
}


string Http_test::readSocket(int socket)
{

}