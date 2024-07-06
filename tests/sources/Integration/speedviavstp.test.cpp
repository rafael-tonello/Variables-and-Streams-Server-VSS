#include "speedviavstp.test.h"

SpeedViaVstpTest::SpeedViaVstpTest()
{
    
}

SpeedViaVstpTest::~SpeedViaVstpTest()
{
    
}


vector<string> SpeedViaVstpTest::getContexts()
{
    return {"speedviavstp"};
}

void SpeedViaVstpTest::run(string context)
{
    if (context != "speedviavstp") return;

    auto logger = new Logger({ 
        new LoggerLambdaWriter([&](Logger* sender, string msg, int level, string name, std::time_t dateTime)
        {
            this->lastLogInfo = {sender, msg, level, name};
            cout << "\t\t\t\t[" << name << "] " << msg << endl;
        })
    });

    IConfProvider *confProvider = new InMemoryConfProvider(
    {
        std::make_tuple("maxTimeWaitingClient_seconds", "10")
    });

    dim.addSingleton<ILogger>(logger);
    dim.addSingleton<ThreadPool>(new ThreadPool(4));
    dim.addSingleton<Confs>(confProvider, {typeid(Confs).name()});
    dim.addSingleton<StorageInterface>(&database);

    
    
}

int SpeedViaVstpTest::connectToServer(string server, int port)
{
    if (auto socketHandle = socket(AF_INET, SOCK_STREAM, 0); socketHandle >= 0)
    {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server.c_str(), &serv_addr.sin_addr) > 0) 
        {
            int cli_fd;
            if ((cli_fd = connect(socketHandle, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) >= 0) 
                return cli_fd;
        }
    }

    return 0;
}