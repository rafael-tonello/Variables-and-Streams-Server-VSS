#ifndef PHOMAU_H
#define PHOMAU_H


#include <pthread.h>
#include <sys/types.h>
#include <iterator>
#include <string.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>

#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "../ApiMediatorInterface.h"
#include "SocketInfo.h"
#include <thread>
#include <logger.h>
 
//#define MSG_DONTWAIT 0x40

#ifdef __TESTING__
    #include <tester.h>
#endif

namespace API {
    using namespace std;

    class PHOMAU_ACTIONS{
    public:
        static string SET_VAR;
        static string GET_VAR;
        static string GET_VAR_RESPONSE;
        static string OBSERVE_VAR;
        static string STOP_OBSERVER_VAR;
        static string VAR_CHANGED;
        static string CREATE_ALIAS;
        static string GET_ALIAS_DESTNAME;
        static string REMOVE_ALIAS;
        static string GET_CHILDS;
        static string GET_CHILDS_RESPONSE;
        static string LOCK_VAR;
        static string UNLOCK_VAR;
        static string LOCK_VAR_DONE;
        static string UNLOCK_VAR_DONE;
    };

    class PHOMAU
    {    
        private:
            #ifdef __TESTING__
                public:
            #endif

            ApiMediatorInterface *ctrl;
            ILogger *log;

            //string __resolveVarName(string key);
            void debug(string msg);

            /**
             * @brief this function is called when a fclient is connected to the server
             * @param client a pointer to an SocketInfo object with the connectd client info
             */
            void clientConnected(SocketInfo* client);

            /**
             * @brief This function is called when a client is disconnected from the server
             * @param cliente is a pointer to an ScoketInfo object with the connected client info
            */
            void clientDisconnected(SocketInfo* client);
            
            /** 
             * @brief this function is called majority by the function ThreadTalkWithClientFunction when a pack is receitved
             * @param data the received data
             * @param the amount of data (number of bytes) received
             * @param clientSocket a SocketInfo object eith information about the socket client
             */
            void __PROCESS_PACK(string command, string varName, char* data, size_t dataSsize, SocketInfo& clientSocket);
            //a helper function to __protocol_phomau_write
            void __PROTOCOL_PHOMAU_WRITE(SocketInfo& clientSocket, string command, string data);

            /** 
             * @brief Mounts a PHOMAU protocol pack and send it to a client socket
             * @param clientSocket A SocketInfo with the scoket client ifnormation who will recieve the data
             * @param command The PHOMAU command. Take a look in the protocol documentation
             * @param data The arguments of the 'command'. Again: See the protocol documentation
             * @param size The size of 'data'
             * 
            */
            void __PROTOCOL_PHOMAU_WRITE(SocketInfo& clientSocket, string command, char* data, unsigned int size);
            void ThreadAwaitClientsFunction();
            void ThreadTalkWithClientFunction(int socketClient);


        public:
            int __port;
            mutex __socketsMutex;
            map<long, SocketInfo*> __sockets;

            PHOMAU(int port, ApiMediatorInterface *ctr, ILogger *log);
            virtual ~PHOMAU();

            bool __SocketIsConnected(SocketInfo socket);
    };

    bool SetSocketBlockingEnabled(int fd, bool blocking);
    //void addStringToCharList(vector<char> *destination, string *source, char*source2, int source2Length = -1);

	enum States {READING_COMMAND, READING_NAME, READING_VALUE, FINISHED};

	enum Protocols{
		P_PHOMAU = 0x02
	};


   
    
}
#endif