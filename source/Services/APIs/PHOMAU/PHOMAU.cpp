#include "PHOMAU.h"

namespace API { 
    PHOMAU::PHOMAU(int port, ApiMediatorInterface *ctr)
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

    PHOMAU::~PHOMAU()
    {
        //dtor
    }

	void PHOMAU::__PROCESS_PACK(char* data, unsigned int size, SocketInfo clientSocket)
	{
        string strData;
        long separatorPosition;
        long observersCount;
        string key;
        string tempStr;
        string value;
        char* buffer;
        bool founded;

        future<tuple<string, DynamicVar>> get_var_fut;
        tuple<string, DynamicVar> get_var_value;

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
                    this->ctrl->getVar(strData, DynamicVar(""));
                    get_var_value = get_var_fut.get();

                    strData = std::get<0>(get_var_value) + "="+(std::get<1>(get_var_value)).getString();


                    /*buffer = new char[strData.size()];
                    for (int cont  =0; cont < strData.size(); cont++)
                        buffer[cont] = strData[cont];*/
                    this->__PROTOCOL_PHOMAU_WRITE(clientSocket, 0x16 , &strData[0], strData.size());
                    //delete[] buffer;
                    //value.clear();
                    //tempStr.clear();

				break;
				case OBSERVE_VAR:
                    //store a clientSocket id in a variable
                    key = strData;
                    this->ctrl->observateVar(key, this, std::to_string(clientSocket.getId()));
                    /*key = this->__resolveVarName(key);
                    tempStr = strData;
                    strData = key + ".observers";
                    observersCount = this->ctrl->GetVar_Int(strData + ".count", 0);
                    //scrolls throught the observers list checking if the client already in this
                    founded = false;
                    for (int cont = 0; cont < observersCount; cont++)
                    {
                        if (this->ctrl->GetVar_Int(strData + "." + std::to_string(cont) + ".id", 0) == clientSocket.getId())
                        {
                            founded = true;
                            break;
                        }
                    }

                    if (!founded)
                    {
                        cout << "New observer to variable " << key <<" added." << endl << flush;
                        this->ctrl->SetVar_Int(strData+"."+std::to_string(observersCount) + ".id", clientSocket.getId());
                        this->ctrl->SetVar(strData+"."+std::to_string(observersCount) + ".observerKeyOrAliasName", tempStr);
                        observersCount++;
                        this->ctrl->SetVar_Int(strData + ".count", observersCount);
                    }

                    //notify new observer with the current value of variable
                    //this->__PROTOCOL_PHOMAU_NOTIFY_OBSERVERS(key, "");
                    value = this->ctrl->GetVar(key + ".value", "");
                    strData = tempStr + "="+value;
                    buffer = new char[strData.size()];
                    for (int cont  =0; cont < strData.size(); cont++)
                        buffer[cont] = strData[cont];

                    this->__PROTOCOL_PHOMAU_WRITE(clientSocket, 0x0E , buffer, strData.size());
                    delete[] buffer;
                    key.clear();
                    value.clear();
                    tempStr.clear();*/
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

                        this->ctrl->SetVar(key + ".value", value);
                        this->ctrl->SetVar(key + ".type", "alias");

                        key.clear();
                        value.clear();
                    }
				break;
				case GET_ALIAS_DESTNAME:

				break;
				case REMOVE_ALIAS:

				break;
                case GET_CHILDS:
                    //string parentName = this->__resolveVarName(strData);
                    string parentName = strData;
                    vector<string> result = this->ctrl->getChildsOfVar(parentName);

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

    /*//scrolls throught the alias until find the desired varname
    string PHOMAU::__resolveVarName(string key)
    {
        string type = this->ctrl->GetVar(key + ".type", "var");
        string handle;
        if (type == "alias")
        {

            type.clear();
            handle = this->ctrl->GetVar(key + ".value", "");

            cout << "Alias found: " << key << " to " << handle << endl << flush;
            if (handle != "")
                return this->__resolveVarName(handle);
            else
                return key;
        }
        else
        {
            type.clear();
            return key;
        }

    }

    void PHOMAU::__PROTOCOL_PHOMAU_NOTIFY_OBSERVERS(string varName, string varValue)
    {
        string currentObserverKeyOrAliasName;

        varName = this->__resolveVarName(varName);

        if (varValue == "")
            varValue = this->ctrl->GetVar(varName + ".value", "");
        //prepares the buffer to be sented to observers
        long currentObserverId;
        SocketInfo currentSocketClient;
        //get the observer count
        long observersCount = this->ctrl->GetVar_Int(varName + ".observers.phomau.count", 0);
        long lastValidObserver = this->ctrl->GetVar_Int(varName + ".observers.phomau.lastValid", 0);
        
        if (observersCount > 0)
            cout << "Alguns observadores serão notificados sobre a variável "<<varName << " (Já foram adicionados "<<observersCount<<" para esta variável, e os "<<lastValidObserver<<" primeiros não são validos)" << endl << flush;
        else
            cout << "A variável "<<varName << " não possui observadores ("<< observersCount<<" adicionados e "<< lastValidObserver << " inválidos)" << endl << flush;
        //scrolls throught the observer to send the notification
        int lastValidObserver_back = lastValidObserver;
        for (int cont = observersCount-1; cont >= lastValidObserver_back; cont--)
        {
            currentObserverId = this->ctrl->GetVar_Int(varName+".observers.phomau."+std::to_string(cont) + ".id", -1);

            currentObserverKeyOrAliasName = this->ctrl->GetVar(varName+".observers.phomau."+std::to_string(cont) + ".observerKeyOrAliasName", varName);


            string strData = currentObserverKeyOrAliasName + "="+varValue;
            char* buffer = new char[strData.size()];
            for (int cont  =0; cont < strData.size(); cont++)
                buffer[cont] = strData[cont];



            //-1 indicates that the client has removed in another operation;
            if (currentObserverId != -1)
            {
                lastValidObserver = cont;

                if (this->__sockets.find(currentObserverId) != this->__sockets.end())
                {
                    currentSocketClient = this->__sockets[currentObserverId];
                    //checks to see if the socket is connected
                    if (this->__SocketIsConnected(currentSocketClient))
                    {
                        this->__PROTOCOL_PHOMAU_WRITE(currentSocketClient, 0x0E , buffer, strData.size());
                    }
                    else
                    {
                        //if the socket is not conencted, delete this from observer list
                        this->ctrl->DelVar(varName+".observers.phomau."+std::to_string(cont) + ".id");
                        this->ctrl->DelVar(varName+".observers.phomau."+std::to_string(cont) + ".observerKeyOrAliasName");
                        this->__sockets.erase(currentObserverId);
                    }

                }
                else
                {
                    this->ctrl->DelVar(varName+".observers.phomau."+std::to_string(cont) + ".id");
                    this->ctrl->DelVar(varName+".observers.phomau."+std::to_string(cont) + ".observerKeyOrAliasName");
                }
            }
                

            //save index of last valid observers found
            this->ctrl->SetVar_Int(varName + ".observers.phomau.lastValid", lastValidObserver);



            strData.clear();
            currentObserverKeyOrAliasName.clear();
            delete[] buffer;

        }
    }*/

    void PHOMAU::__PROTOCOL_PHOMAU_WRITE(SocketInfo clientSocket, char command, char* data, unsigned int size)
    {
        size = size + 1; //size of command need to be sented in "size" field
        char* writeBuffer = new char[size + 13];
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

        int sented = send(clientSocket.socket, writeBuffer, size + 13, MSG_NOSIGNAL);
        if (sented < 0)
        {
            //disconnect the client
        }

        //clear the data
        delete[] writeBuffer;

    }

    void PHOMAU::ThreadAwaitClientsFunction()
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
            setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &iOptVal, &iOptLen);
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
                            thread th(() = {
                                thTalkWithClient(client);
                            });
                            th.detach();
                            usleep(1000);
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

    void PHOMAU::ThreadTalkWithClientFunction(int socketClient)
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

        States oldState = States::FINISHED;
        while (state != States::FINISHED)
        {

            /*#ifdef __WIN32__
            ioctlsocket(client, FIONREAD, &ToRead);
            #else*/
            ioctl(client->socket,FIONREAD,&ToRead);
            // #endif

            this->usleep(50000);
            if (this->__SocketIsConnected(*client))
            {
                if (ToRead > 0)
                {
                    tempBuffer = new char[ToRead+1];
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
    									this->__PROTOCOL_PHOMAU(data, dataSize, *client);
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

		close(client->socket);
        if (this->__sockets.find(client->getId()) != this->__sockets.end())
        {
            this->__sockets.erase(client->getId());
        }

        //headerLines.clear();
        //bufferStr.clear();
        delete[] params;
        params = NULL;

        if (tempBuffer)
            delete[] tempBuffer;
         tempBuffer = NULL;


        //pthread_detach(*thTalkWithClient);
        pthread_exit(0);
    }

    bool SetSocketBlockingEnabled(int fd, bool blocking)
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

    /*void PHOMAU::USleep(long MicroSeconds)
    {
        timespec tim;
        timespec remainOnEnd;
        tim.tv_sec  = 0;
        tim.tv_nsec = MicroSeconds * 1000;

        nanosleep(&tim, &remainOnEnd);

    }*/

    bool PHOMAU::__SocketIsConnected(SocketInfo socket)
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
            socket.upadateKeepAlive();

        int keepAliveSeconds = socket.getKeepAliveSeconds();

        //bool connected = keepAliveSeconds <= (15*60);
        bool connected = keepAliveSeconds <= (60*60);
        if (!connected)
            cout << "Um client foi desconectado" << endl << flush;
        return connected;
    }
}