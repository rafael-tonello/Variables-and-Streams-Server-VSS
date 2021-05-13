//#ifndef PHOMAU_H
//#define PHOMAU_H


#include <pthread.h>
#include <sys/types.h>
#include <iterator>
#include <string.h>
#include <string>
#include <list>
#include <vector>
#include <map>
//#include "StringUtils.h"
#include <fcntl.h>
#include <iostream>
#include <stdio.h>

#ifdef __WIN32__
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#endif

#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "../ApiMediatorInterface.h"
#include "SocketInfo.h"
#include <thread>
 
//#define MSG_DONTWAIT 0x40

#ifdef __TESTING__
    #include <tester.h>
#endif

namespace API {
    using namespace std;

    class PHOMAU
    {    
        private:
            #ifdef __TESTING__
                public:
            #endif

            ApiMediatorInterface *ctrl;

            //string __resolveVarName(string key);
            void debug(string msg){cout << msg << endl;}

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
            void __PROCESS_PACK(char* data, unsigned int size, SocketInfo clientSocket);

            /**
             * @brief Mounts a PHOMAU protocol pack and send it to a client socket
             * @param clientSocket A SocketInfo with the scoket client ifnormation who will recieve the data
             * @param command The PHOMAU command. Take a look in the protocol documentation
             * @param data The arguments of the 'command'. Again: See the protocol documentation
             * @param size The size of 'data'
             * 
            */
            void __PROTOCOL_PHOMAU_WRITE(SocketInfo clientSocket, char command, char* data, unsigned int size);
            void ThreadAwaitClientsFunction();
            void ThreadTalkWithClientFunction(int socketClient);
        public:
            int __port;
            map<long, SocketInfo> __sockets;

            PHOMAU(int port, ApiMediatorInterface *ctr);
            virtual ~PHOMAU();

            bool __SocketIsConnected(SocketInfo socket);
    };

    bool SetSocketBlockingEnabled(int fd, bool blocking);
    //void addStringToCharList(vector<char> *destination, string *source, char*source2, int source2Length = -1);

	enum States {AWAIT_HEADER, READING_PROTOCOL_TYPE, READING_DATA_SIZE, READING_DATA, IDENTIFYING_PROTOCOL, FINISHED};

	enum Protocols{
		P_PHOMAU = 0x02
	};

	enum PhomauCommands{
		ACKNOWNLEDGE = 0x01,
        KEEP_ALIVE = 0x02,
		SET_VAR = 0x0A,
		GET_VAR = 0x0B,
		OBSERVE_VAR = 0x0C,
		STOP_OBSERVER_VAR = 0x0D,
		VAR_CHANGED = 0x0E,
		CREATE_ALIAS = 0x0F,
		GET_ALIAS_DESTNAME = 0x10,
		REMOVE_ALIAS = 0x11,
        KEEP_ALIVE_2 = 0x12,
        GET_CHILDS = 0x17,
        GET_CHILDS_RESPONSE = 0x18,
        APPEND_VAR = 0x15,
        //GET_VAR_RESPONSE = 0x16,
        GET_VAR_SIZE = 0x19,
        GET_VAR_SIZE_RESPONSE = 0x1A,
        GET_VAR_SIZE_BIN = 0x1B,
        GET_VAR_SIZE_BIN_RESPONSE = 0x1C,
        ATTACHED_ON_VAR = 0x1D //APPENDED TO VAR
	};
}
//#endif