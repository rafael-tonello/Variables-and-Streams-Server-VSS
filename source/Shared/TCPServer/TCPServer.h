//TODO: Create a new socket system using libuv: docs.libuv.org/en/v1.x/guide.html


#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <functional>
#include <map>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include "../ThreadPool/ThreadPool.h"
#include <sstream>
#pragma region include for networking
    #include <sys/types.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <errno.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#pragma endregion

namespace TCPServerLib
{
    using namespace std;
    class TCPServer;

    enum CONN_EVENT{CONNECTED, DISCONNECTED};
    
    class ClientInfo;

    class SocketHelper{
        protected:
            vector<function<void(ClientInfo *client, char* data,  size_t size)>> receiveListeners;
            vector<function<void(ClientInfo *client, string data)>> receiveListeners_s;
            vector<function<void(ClientInfo *client, CONN_EVENT event)>> connEventsListeners;
        public:
            int socketHandle;
            map<string, void*> tags;

            void addReceiveListener(function<void(ClientInfo *client, char* data,  size_t size)> onReceive);
            void addReceiveListener_s(function<void(ClientInfo *client, string data)> onReceive);
            void addConEventListeners(function<void(ClientInfo *client, CONN_EVENT event)> onConEvent);

            void* __getPrivate(string n){
                if (n == "receiveListeners")
                    return (void*)&this->receiveListeners;
                else if (n == "receiveListeners_s")
                    return (void*)&this->receiveListeners_s;
                else if (n == "connEventsListeners")
                    return (void*)&this->connEventsListeners;
            }
    };

    class ClientInfo: public SocketHelper{
        public:
            mutex writeMutex;
            ClientInfo();
            TCPServer *server;
            ClientInfo(TCPServer *server);
            void send(char* data, size_t size);
            void send(string data);
            bool isConnected();
    };

    

    class TCPServer: SocketHelper{
        private:
            ThreadPool *__tasks = NULL;
            map<int, ClientInfo> connectedClients;
            vector<thread*> listenThreads;
            void notifyListeners_dataReceived(ClientInfo *client, char* data, size_t size);
            void notifyListeners_connEvent(ClientInfo *client, CONN_EVENT action);
            void initialize(vector<int> ports, ThreadPool *tasker = NULL);
            void listenClients(int port);
            void debug(string msg){cout << "Debug: " << msg << endl;}
            void ChatWithClient(ClientInfo *client);
            bool __SocketIsConnected( int socket);
            void talkWithClients();
        public:
            map<string, void*> tags;
            
            TCPServer(int port, ThreadPool *tasker = NULL);
            TCPServer(vector<int> ports, ThreadPool *tasker = NULL);
            ~TCPServer();

            void send(ClientInfo *client, char* data, size_t size);
            void send(ClientInfo *client, string data);
            void sendBroadcast(char* data, size_t size, vector<ClientInfo*> *clientList = NULL);
            void sendBroadcast(string data, vector<ClientInfo*> *clientList = NULL);

    };
}
#endif