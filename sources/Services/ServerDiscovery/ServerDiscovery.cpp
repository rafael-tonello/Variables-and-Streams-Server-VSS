#include  "ServerDiscovery.h" 



ServerDiscovery::ServerDiscovery(DependencyInjectionManager &dim, string version)
{
    this->dim = &dim;
    this->serverVersion = version;
    this->log = dim.get<ILogger>()->getNamedLoggerP("ServerDiscovery");
    
    thread th([&](){ this->run();});
    th.detach();
}

ServerDiscovery::~ServerDiscovery() 
{ 
    running = false;
    usleep(10000);
    delete this->log;
}

void ServerDiscovery::run()
{
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
 
    socklen_t cliLen = sizeof(serv_addr);

    char buffer[255] = {0};

	
	//int s, i, slen = sizeof(si_other) , recv_len;
    int listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (listener >= 0)
    {
        int reuse = 1;
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
            log->error("setsockopt(SO_REUSEADDR) failed");

        /*int broadcastEnable=1;
        if(setsockopt(listener, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) <0)
            log->error("setsockopt(SO_BROADCAST) failed");*/

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //htonl(INADDR_BROADCAST); //htonl(INADDR_ANY)
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
                    string cliIp = string(inet_ntoa(cli_addr.sin_addr));
                    log->info2("Received "+to_string(readCount) + " bytes from "+cliIp+".");
                    
                    if (IsAServerSearchMessage(string(buffer, readCount)))
                    {
                        log->info("Received a valid message from "+ cliIp +": "+string(buffer, readCount));
                        string response = getServerInfo(cliIp);
                        response += "\n";

                        log->info("Sending a server info message to "+cliIp+": "+response);
                        sendto(listener, response.c_str(), response.size(), 0, (struct sockaddr*) &cli_addr, cliLen);
                    }

                    for (uint c = 0; c < 255; c++)
                        buffer[c] = 0;
                }
                usleep(1000);
            }
        }
        else
        {
            log->error("Error binding socket. The Server Discovery service is not running");
            this->port = -1;
        }
    }
    else
    {
        log->error("Error starting socket. The Server Discovery service is not running");
        this->port = -1;
    }

	close(listener);
}

bool ServerDiscovery::IsAServerSearchMessage(string message)
{
    return message.find("where is vss?") != string::npos || message.find("wiv") != string::npos;
}

string ServerDiscovery::getServerInfo(string clientIp)
{
    auto ips = getServerIps();
    auto apis = getApisInfo();

    JSON js;
    js.setString("vssServerInfo.version", this->serverVersion);


    //detect the most problame ip
    string tmp = getMostProbaleIp(ips, clientIp);
    if (tmp != "")
        js.setString("vssServerInfo.mainIp", tmp);

    int i = 0;
    for (auto &c: ips)
        js.setString(Utils::sr("vssServerInfo.ips[$i]", "$i", to_string(i++)), c);

    i = 0;
    for (auto &c: apis)
    {
        js.setString(Utils::sr("vssServerInfo.apis[$i].name", "$i", to_string(i)), c["name"]);
        js.setString(Utils::sr("vssServerInfo.apis[$i].access", "$i", to_string(i)), c["access"]);
        i++;
    }

    return js.ToJson(true);
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

vector<map<string, string>> ServerDiscovery::getApisInfo()
{
    vector<map<string, string>> result;
    auto ret = dim->get<MessageBus<JsonMaker::JSON>>()->post("discover.startedApis", JSON());
    for (auto &c: ret)
    {
        map<string, string> tmp;
        tmp["name"] = c.getString("name", "");
        tmp["access"] = c.getString("access", "");

        result.push_back(tmp);
    }

    return result;
}

int ServerDiscovery::getStringCompatibility(string string1 , string string2)
{
    size_t ret =0;
    while (ret< string1.size() && ret < string2.size() && string1[ret] == string2[ret])
        ret++;

    return (int)ret;
}

string ServerDiscovery::getMostProbaleIp(vector<string> ips, string clientIp)
{
    string ret = "";
    int bestCompatibility = 0;

    for (auto &c: ips)
    {
        int currCompatibility = getStringCompatibility(c, clientIp);
        if (currCompatibility > bestCompatibility)
        {
            bestCompatibility = currCompatibility;
            ret = c;
        }
    }

    return ret;
}

string ServerDiscovery::getRunningPortInfo()
{
    if (this->port > -1)
        return "UDP/"+to_string(port);
    else
        return "Error - No opened port";
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

//for tests
//echo 'wiv' | socat - udp-datagram:192.168.100.255:5022,bind=:5022,broadcast,reuseaddr
//echo 'wiv' | socat -t 5 - udp-datagram:192.168.100.255:5022,bind=:5022,broadcast,reuseaddr
//https://superuser.com/questions/1473729/send-udp-packet-and-listen-for-replies