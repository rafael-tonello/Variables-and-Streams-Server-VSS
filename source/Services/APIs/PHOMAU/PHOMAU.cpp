#include "PHOMAU.h"


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

void API::PHOMAU::__PROCESS_PACK(char* data, unsigned int size, SocketInfo clientSocket)
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

    if (size > 1)
    {
        for (unsigned int cont = 1; cont< size; cont++)
            strData += data[cont];

        switch (data[0])
        {
            //case ACKNOWNLEDGE:

            //break;
            case KEEP_ALIVE:
            case KEEP_ALIVE_2:
                //send back to client the same KEEP_ALIVE pack
                //this->__PROTOCOL_PHOMAU_WRITE(clientSocket, data[0], new char[0], 0);

            break;
            case SET_VAR:
                separatorPosition = strData.find("=");
                if (separatorPosition > -1)
                {
                    string key = strData.substr(0, separatorPosition);
                    string value = strData.substr(separatorPosition+1, string::npos);
                    this->ctrl->setVar(key, value);

                    //key.clear();
                    //value.clear();
                }
            break;
            case GET_VAR:
                //store a clientSocket id in a variable
                get_var_fut = this->ctrl->getVar(strData, DynamicVar(string("")));
                get_var_values = get_var_fut.get();
                

                for (auto &c : get_var_values)
                {
                    strData = std::get<0>(c) + "="+(std::get<1>(c)).getString();

                    this->__PROTOCOL_PHOMAU_WRITE(clientSocket, 0x16 , &strData[0], strData.size());
                }
                //delete[] buffer;
                //value.clear();
                //tempStr.clear();

            break;
            case OBSERVE_VAR:
                //store a clientSocket id in a variable
                key = strData;

                this->ctrl->observeVar(key, [&](string name, DynamicVar value, void* args, string id)
                {
                    return;
                    //TODO: nofity client
                    

                }, (void*)NULL, std::to_string(clientSocket.getId()));
                
            break;
            case STOP_OBSERVER_VAR:

            break;
            //case VAR_CHANGED:
            //
            //break;
            case CREATE_ALIAS:
                separatorPosition = strData.find("=");
                if (separatorPosition > -1)
                {
                    string key = strData.substr(0, separatorPosition);
                    string value = strData.substr(separatorPosition+1, string::npos);

                    this->ctrl->setVar(key + ".value", DynamicVar(string(value)));
                    this->ctrl->setVar(key + ".type", DynamicVar(string("alias")));
                }
            break;
            case GET_ALIAS_DESTNAME:

            break;
            case REMOVE_ALIAS:

            break;
            case GET_CHILDS:
                //string parentName = this->__resolveVarName(strData);
                string parentName = strData;
                vector<string> result = this->ctrl->getChildsOfVar(parentName).get();

                //preprare response csv
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

                this->__PROTOCOL_PHOMAU_WRITE(clientSocket, GET_CHILDS_RESPONSE, buffer, response.size());

                //clear used data
                delete[] buffer;
                parentName.clear();
                result.clear();
                response.clear();

            break;
        }
    }

    strData.clear();
}



void API::PHOMAU::__PROTOCOL_PHOMAU_WRITE(SocketInfo clientSocket, char command, char* data, unsigned int size)
{
    size = size + 1; //size of command need to be sented in "size" field
    int bufSize = size+12;
    char* writeBuffer = new char[bufSize];
    unsigned int writeBufferIndex = 0;
    writeBuffer[writeBufferIndex++]=0x02; //stx
    writeBuffer[writeBufferIndex++]=0x02; ;//PHOMAU protocol
    //the size (unsigned long)


    //data size format 1 (most significant byte at left)
    writeBuffer[writeBufferIndex++]=((char*)(&size))[0];
    writeBuffer[writeBufferIndex++]=((char*)(&size))[1];
    writeBuffer[writeBufferIndex++]=((char*)(&size))[2];
    writeBuffer[writeBufferIndex++]=((char*)(&size))[3];

    //datasize format2 (most significant byte at right)
    writeBuffer[writeBufferIndex++]=((char*)(&size))[3];
    writeBuffer[writeBufferIndex++]=((char*)(&size))[2];
    writeBuffer[writeBufferIndex++]=((char*)(&size))[1];
    writeBuffer[writeBufferIndex++]=((char*)(&size))[0];

    //the command
    writeBuffer[writeBufferIndex++] = command;

    //the data
    for (unsigned int cont = 0; cont < size-1; cont++){ //-1 is because that command is sented separated (before the data field), but his size is increased in size
        writeBuffer[writeBufferIndex++]=data[cont];
    }

    //the crc (not used)
    writeBuffer[writeBufferIndex++]=0x00;
    writeBuffer[writeBufferIndex++]=0x00;
    //writes the pack to socket client

    int sented = send(clientSocket.socket, writeBuffer, bufSize, MSG_NOSIGNAL);


        
    #ifdef __TESTING__
        Tester::global_test_result.resize(bufSize);
        for (size_t c = 0; c < bufSize; c++)
            Tester::global_test_result[c] = writeBuffer[c];
    #endif

    if (sented < 0)
    {
        //disconnect the client
    }

    //clear the data
    delete[] writeBuffer;

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
                        //thTalkWithClient = new pthread_t;
                        //tmp = new void*[3];
                        //tmp[0] = self;
                        //tmp[1] = &client;
                        //tmp[2] = thTalkWithClient;
                        //pthread_create(thTalkWithClient, NULL, ThreadTalkWithClientFunction, tmp);
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
    //vector<char> rawBuffer;
    int readed = 1;

    char *data = NULL;

    char curr;
    unsigned int currentPackageIndex = 0;
    unsigned int tempCount;



    //the data of package
    char protocolType;
    unsigned long dataSize;
    char* dataSizeAsBuffer = (char*)&dataSize;

    States state = States::AWAIT_HEADER;
    int ToRead;

    unsigned int currCharIndex = 0;

    int val;
    //setsockopt(client, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof val);

    this->clientConnected(client);

    States oldState = States::FINISHED;
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
                        currentPackageIndex++;

                        // cout << (int)curr << " ("<< (char)curr <<")" << endl << flush;
                        //
                        //
                        // if (oldState != state)
                        // {
                        //     cout << "The current state is " << state << endl << flush;
                        // }

                        oldState = state;

                        switch(state)
                        {
                            case AWAIT_HEADER:
                                    if (curr == 0x02)
                                    {
                                        state = READING_PROTOCOL_TYPE;
                                        currentPackageIndex = 0;
                                    }
                            break;
                            case READING_PROTOCOL_TYPE:
                                protocolType = curr;

                                tempCount = 0;
                                state = READING_DATA_SIZE;
                                break;
                            case READING_DATA_SIZE:
                                if (tempCount < 4)
                                    dataSizeAsBuffer[tempCount] = curr;
                                tempCount++;
                                if (tempCount == 8)
                                {
                                    state = READING_DATA;
                                    tempCount = 0;
                                    data = new char[dataSize];
                                }
                                break;
                            case READING_DATA:
                                data[tempCount++] = curr;
                                if (tempCount == dataSize)
                                {
                                    state = IDENTIFYING_PROTOCOL;
                                    tempCount = 0;
                                }
                                break;
                            case IDENTIFYING_PROTOCOL:
                                if (protocolType == P_PHOMAU)
                                {
                                    this->__PROCESS_PACK(data, dataSize, *client);
                                }

                                //clear all
                                delete[] data;
                                data = NULL;

                                state = AWAIT_HEADER;
                                break;
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
