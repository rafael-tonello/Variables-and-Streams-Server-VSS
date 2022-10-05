#include  "ServerDiscovery.h" 



ServerDiscovery::ServerDiscovery(DependencyInjectionManager &dim)
{
    auto ips = getServerInfo();
    this->log = dim.get<ILogger>()->getNamedLoggerP("ServerDiscovery");
    thread th([&](){ this->run();});
    th.detach();
}

ServerDiscovery::~ServerDiscovery() 
{ 
    running = false;
    delete this->log;
}

void ServerDiscovery::run()
{
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;

    socklen_t cliLen = sizeof(cli_addr);

    char buffer[255] = {0};

	
	//int s, i, slen = sizeof(si_other) , recv_len;
    int listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (listener >= 0)
    {
        int reuse = 1;
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
            cout << "setsockopt(SO_REUSEADDR) failed" << endl;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY; //htonl(INADDR_ANY)
        serv_addr.sin_port = htons(port);

        int status = bind(listener, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        if (status >= 0)
        {
            SetSocketBlockingEnabled(listener, false);
            //status = listen(listener, 5);
            log->info("Server discovery is running on UDP port " + to_string(port));
            while (running)
            {

                int readCount = recvfrom(listener, buffer, 255, 0, (struct sockaddr *) &cli_addr, &cliLen);
                if (readCount > 0)
                {
                    if (IsAServerSearchMessage(string(buffer, readCount)))
                    {
                        string response = getServerInfo();
                        sendto(listener, response.c_str(), response.size(), 0, (struct sockaddr*) &cli_addr, cliLen);
                    }

                    for (uint c = 0; c < 255; c++)
                        buffer[c] = 0;
                }
                usleep(10000);
            }
        }
        else
            log->error("Error binding socket");
    }
    else
    {
        log->error("Error starting socket");
    }

	close(listener);
}

bool ServerDiscovery::IsAServerSearchMessage(string message)
{
    return message == "where is vss?" || message == "wiv";
}

string ServerDiscovery::getServerInfo()
{
    auto ips = getServerIps();
    string info = "I'm here. Ips:\n";
    for (auto &c: ips)
        info += c + "\n";

    return info;
}
 
bool ServerDiscovery::SetSocketBlockingEnabled(int fd, bool blocking)
{
    if (fd < 0) return false;

    #ifdef _WIN32
        unsigned long mode = blocking ? 0 : 1;
        return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
    #else
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) return false;
        flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
        return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
    #endif
}

vector<string> ServerDiscovery::getServerIps()
{
    string cmdResult = ssystem("ip route");
    vector<string> result;
    size_t pos = cmdResult.find("src ");
    while (pos != string::npos)
    {
        cmdResult = cmdResult.substr(pos+4);
        string currIp = cmdResult.substr(0, cmdResult.find(" "));

        result.push_back(currIp);
        pos = cmdResult.find("src ");
    }

    return result;
}

string ServerDiscovery::ssystem (string command, bool removeTheLastLF) {
    //todo:change it to use popen (https://stackoverflow.com/questions/45202379/how-does-popen-work-and-how-to-implement-it-into-c-code-on-linux)
    char tmpname [L_tmpnam];
    std::tmpnam ( tmpname );

    char tmpname2 [L_tmpnam];
    std::tmpnam ( tmpname2 );
    std::string scommand = command;
    std::string cmd = scommand + " > " + tmpname;

    Utils::writeTextFileContent(string(tmpname2) + ".sh", cmd);

    auto pid = fork();

    if (pid < 0)
    {

    }
    else if (pid == 0)
    {
        execlp("chmod", "chmod", "+x", (string(tmpname2)+".sh").c_str(), NULL);

        exit(0);
    }

    waitpid(pid, 0, 0);


    pid = fork();

    if (pid < 0)
    {

    }
    else if (pid == 0)
    {
        execlp((string(tmpname2)+".sh").c_str(), (string(tmpname2)+".sh").c_str(), NULL);
        exit(0);
    }

    waitpid(pid, 0, 0);



    std::ifstream file(tmpname, std::ios::in | std::ios::binary );
    std::string result;
    if (file) {
        while (!file.eof()) result.push_back(file.get())
            ;
        file.close();
    }
    remove(tmpname);
    remove(tmpname2);
    
    //remove the last character, with is comming with invalid value
    result = result.substr(0, result.size()-1);

    if (removeTheLastLF)
        result = result.substr(0, result.size()-1);

    return result;
}