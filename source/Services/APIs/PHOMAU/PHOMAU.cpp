#include "PHOMAU.h"

string API::PHOMAU_ACTIONS::SET_VAR = "sv";
string API::PHOMAU_ACTIONS::GET_VAR = "gv";
string API::PHOMAU_ACTIONS::GET_VAR_RESPONSE = "gvr";
string API::PHOMAU_ACTIONS::OBSERVE_VAR = "ov";
string API::PHOMAU_ACTIONS::STOP_OBSERVER_VAR = "sov";
string API::PHOMAU_ACTIONS::VAR_CHANGED = "vc";
string API::PHOMAU_ACTIONS::CREATE_ALIAS = "ca";
string API::PHOMAU_ACTIONS::GET_ALIAS_DESTNAME = "gad";
string API::PHOMAU_ACTIONS::REMOVE_ALIAS = "ra";
string API::PHOMAU_ACTIONS::GET_CHILDS = "gc";
string API::PHOMAU_ACTIONS::GET_CHILDS_RESPONSE = "gcr";


API::PHOMAU::PHOMAU(int port, ApiMediatorInterface *ctr)
{
    this->__port = port;
    this->ctrl = ctr;
    //ctorDD
    /*#ifdef __WIN32__
        WORD versionWanted = MAKEWORD(1, 1);
        WSADATA wsaData;
        WSAStartup(versionWanted, &wsaData);
    #endif*/

    //pthread_create(&(this->ThreadAwaitClients), NULL, ThreadAwaitClientsFunction, this);

    thread th([this](){
        this->ThreadAwaitClientsFunction();
    });
    th.detach();

    
}

API::PHOMAU::~PHOMAU()
{
    //dtor
}

void API::PHOMAU::__PROCESS_PACK(string command, string varName, char* data, size_t dataSize, SocketInfo clientSocket)
{
    string strData;
    long separatorPosition;
    long observersCount;
    string key;
    string tempStr;
    string value;
    char* buffer;
    bool founded;

    future<vector<tuple<string, DynamicVar>>> get_var_fut;
    vector<tuple<string, DynamicVar>> get_var_values;

    if (command ==PHOMAU_ACTIONS::SET_VAR)
    {
        string value = string(data, dataSize);
        this->ctrl->setVar(varName, value);
    }
    else if (command == PHOMAU_ACTIONS::GET_VAR)
    {
        //store a clientSocket id in a variable
        get_var_fut = this->ctrl->getVar(varName, DynamicVar(string("")));
        get_var_values = get_var_fut.get();
        

        for (auto &c : get_var_values)
        {
            //strin as buffer. I know, this is is not a good praticy.. May be i change this sometime
            string buffer = std::get<0>(c) + "="+(std::get<1>(c)).getString();
            this->__PROTOCOL_PHOMAU_WRITE(clientSocket, PHOMAU_ACTIONS::GET_VAR_RESPONSE , &buffer[0], buffer.size());
        }
    }
    else if (command == PHOMAU_ACTIONS::OBSERVE_VAR)
    {

            this->ctrl->observeVar(varName, [&](string name, DynamicVar value, void* args, string id)
            {
                return;
                //TODO: nofity client
                

            }, (void*)NULL, std::to_string(clientSocket.getId()));
    }
    else if (command == PHOMAU_ACTIONS::STOP_OBSERVER_VAR)
    {

    }
    else if (command == PHOMAU_ACTIONS::CREATE_ALIAS)
    {
        string destination = string(data, dataSize);
        this->ctrl->createAlias(varName, destination);
    }
    else if (command ==  PHOMAU_ACTIONS::GET_ALIAS_DESTNAME)
    {

    }
    else if (command == PHOMAU_ACTIONS::REMOVE_ALIAS)
    {

    }
    else if (command == PHOMAU_ACTIONS::GET_CHILDS)
    {
        vector<string> result = this->ctrl->getChildsOfVar(varName).get();
        string response = "";
        for (int cont = 0; cont < result.size(); cont++)
        {
            response += result[cont];
            if (cont < result.size()-1)
                response += ",";
            
        }

        //prepara buffer with response and send this
        buffer = new char[response.size()];
        for (int cont  =0; cont < response.size(); cont++)
            buffer[cont] = response[cont];

        this->__PROTOCOL_PHOMAU_WRITE(clientSocket, PHOMAU_ACTIONS::GET_CHILDS_RESPONSE, buffer, response.size());

        //clear used data
        delete[] buffer;
        result.clear();
        response.clear();
    }
}



void API::PHOMAU::__PROTOCOL_PHOMAU_WRITE(SocketInfo clientSocket, string command, char* data, unsigned int size)
{
    string sendBuffer = command + ":";
    for (size_t c = 0; c < size; c++)
        sendBuffer += data[c];


    int sented = send(clientSocket.socket, &sendBuffer[0], sendBuffer.size(), MSG_NOSIGNAL);
        
    #ifdef __TESTING__
        Tester::global_test_result = sendBuffer;
    #endif

    if (sented < 0)
    {
        //disconnect the client
    }
}

void API::PHOMAU::ThreadAwaitClientsFunction()
{

    //create an socket to await for connections

    int listener;

    struct sockaddr_in *serv_addr = new sockaddr_in();
    struct sockaddr_in *cli_addr = new sockaddr_in();
    int status;
    socklen_t clientSize;
    pthread_t *thTalkWithClient = NULL;



    listener = socket(AF_INET, SOCK_STREAM, 0);

    //void ** tmp;

    if (listener >= 0)
    {
        //fill(std::begin(serv_addr), std::end(serv_addr), T{});
        //bzero((char *) &serv_addr, sizeof(serv_addr));
        int reuse = 1;
        
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) < 0)
            debug("Cant't set socket option REUSEADDR");

        serv_addr->sin_family = AF_INET;
        serv_addr->sin_addr.s_addr = INADDR_ANY;
        serv_addr->sin_port = htons(this->__port);

        status = bind(listener, (struct sockaddr *) serv_addr, sizeof(*serv_addr));
        if (status >= 0)
        {
            status = listen(listener, 5);
            if (status >= 0)
            {
                clientSize = sizeof(cli_addr);
                cout << "Server is waiting for incoming connections on port "<< this->__port << endl << flush;
                while (true)
                {
                    int client = accept(listener, (struct sockaddr *) cli_addr, &clientSize);
                    if (client >= 0)
                    {
                        thread th([client, this](){
                            ThreadTalkWithClientFunction(client);
                        });
                        th.detach();
                        //usleep(1000);
                    }
                    else
                        usleep(10000);
                }
            }
            else
                cout << "Problem to open socket" << endl << flush;
        }
        else
            cout << "Problem to bind socket" << endl << flush;
    }
    else
        cout << "Some error with socklen_t" << endl << flush;

        //n = read(newsockfd,buffer,255);
        //n = write(newsockfd,"I got your message",18);
}

void API::PHOMAU::ThreadTalkWithClientFunction(int socketClient)
{
    SocketInfo *client = new SocketInfo();
    client->socket = socketClient;
    this->__sockets[client->getId()] = *client;
    SetSocketBlockingEnabled(client->socket, true);


    char *tempBuffer = NULL ;// = new char[2048];
    int readed = 1;
    char curr;
    States state = States::READING_COMMAND;
    int ToRead;
    string command = "";
    string name = "";
    vector<char> value;
    bool scape = false;
    

    unsigned int currCharIndex = 0;
    //setsockopt(client, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof val);
    //this->clientConnected(client);
    while (state != States::FINISHED)
    {

        /*#ifdef __WIN32__
        ioctlsocket(client, FIONREAD, &ToRead);
        #else*/
        ioctl(client->socket,FIONREAD,&ToRead);
        // #endif

        usleep(50000);
        if (this->__SocketIsConnected(*client))
        {
            if (ToRead > 0)
            {
                tempBuffer = new char[ToRead];
                //tempBuffer[ToRead] = 0;

                readed = recv(client->socket, tempBuffer, ToRead, 0);//sizeof(char));

                if (readed > 0)
                {
                    //bufferStr.append(tempBuffer);
                    for (int cont = 0; cont < readed; cont++)
                    {

                        curr = tempBuffer[cont];

                        switch(state)
                        {
                            case READING_COMMAND:
                                    if (curr == '\n')
                                    {
                                        command = "";
                                        name = "";
                                        value.clear();
                                    }
                                    else if (curr == ':')
                                        state = READING_NAME;
                                    else
                                        command += curr;
                            break;
                            case READING_NAME:
                                if (curr == '\n')
                                {
                                    __PROCESS_PACK(command, name, "", 0, *client);
                                    command = "";
                                    name = "";
                                    value.clear();
                                    state = READING_COMMAND;
                                }
                                else if (curr == '=')
                                    state = READING_VALUE;
                                else
                                    name += curr;
                            break;
                            case READING_VALUE:
                                if (curr == '\n')
                                {
                                    __PROCESS_PACK(command, name, value.data(), value.size(), *client);
                                    command = "";
                                    name = "";
                                    value.clear();
                                    state = READING_COMMAND;
                                }
                                else
                                {
                                    //lead with some scaep chars
                                    if (scape)
                                    {
                                        if (curr == '\\')
                                            value.push_back('\\');
                                        else if (curr == 'n')
                                            value.push_back('\n');
                                        else 
                                            value.push_back(curr);

                                        scape = false;
                                    }
                                    else if (curr == '\\')
                                        scape = true;
                                    else
                                        value.push_back(curr);
                                }
                            break;
                        }
                    }

                    //clear all
                    delete[] tempBuffer;
                    tempBuffer = NULL;

                    //libera o buffer depois de usar, jusé
                }
                else if (readed <= 0)
                {
                    //socket disconnected
                    state = States::FINISHED;
                }
                else
                {
                    usleep(50000);
                }
            }
            else if (ToRead < 0)
            {
                //socket disconnected
                state = States::FINISHED;
            }
            else
                //usleep(10000);
                usleep(50000);
        }
        else
        {
            state = States::FINISHED;
        }
    }

    cout << "Saiu do While, state = " << state <<endl  << flush;

    this->clientDisconnected(client);

    close(client->socket);
    //warning: concurrent problem in this if (two or more thread can try to erase elements from this->__sockets)
    if (this->__sockets.find(client->getId()) != this->__sockets.end())
    {
        this->__sockets.erase(client->getId());
    }

    if (tempBuffer)
        delete[] tempBuffer;
    //tempBuffer = NULL;
    
    delete client;


    //pthread_detach(*thTalkWithClient);
    pthread_exit(0);
}

bool API::SetSocketBlockingEnabled(int fd, bool blocking)
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

bool API::PHOMAU::__SocketIsConnected(SocketInfo socket)
{
    char data;
    int readed = recv(socket.socket,&data,1, MSG_PEEK | MSG_DONTWAIT);//read one byte (but not consume this)

    int error_code;
    socklen_t error_code_size = sizeof(error_code);
    getsockopt(socket.socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    string desc(strerror(error_code));

    if (error_code != 0)
        return false;

    return error_code == 0;

    if (readed > 0)
        socket.upadateAlive();

    int keepAliveSeconds = socket.getAliveSeconds();

    //bool connected = keepAliveSeconds <= (15*60);
    bool connected = keepAliveSeconds <= (60*60);
    if (!connected)
        cout << "Um client foi desconectado" << endl << flush;
    return connected;
}

void API::PHOMAU::clientConnected(SocketInfo* client)
{
    #ifdef __TESTING__
        Tester::msgBusNotify("API::PHOMAU::clientConnected called", "", (void*)client);
    #endif
}

void API::PHOMAU::clientDisconnected(SocketInfo* client)
{
    #ifdef __TESTING__
        Tester::msgBusNotify("API::PHOMAU::clientDisconnected called", "", (void*)client);
    #endif

}
