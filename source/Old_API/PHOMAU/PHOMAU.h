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

//#define MSG_DONTWAIT 0x40

namespace API {
    using namespace std;

    class PHOMAU
    {
        private:
            ApiMediatorInterface *ctrl;

            string __resolveVarName(string key);
        public:
            int __port;
            map<long, SocketInfo> __sockets;

            PHOMAU(int port, ApiMediatorInterface *ctr);
            virtual ~PHOMAU();

            void USleep(long MicroSeconds);

            void __PROTOCOL_PHOMAU(char* data, unsigned int size, SocketInfo clientSocket);
            void __PROTOCOL_PHOMAU_WRITE(SocketInfo clientSocket, char command, char* data, unsigned int size);
            void __PROTOCOL_PHOMAU_NOTIFY_OBSERVERS(string varName, string varValue);
            bool __SocketIsConnected(SocketInfo socket);

        protected:

        private:
            pthread_t ThreadAwaitClients;
    };

    void *ThreadAwaitClientsFunction(void *thisPointer);
    void *ThreadTalkWithClientFunction(void *arguments);
    bool SetSocketBlockingEnabled(int fd, bool blocking);
    void addStringToCharList(vector<char> *destination, string *source, char*source2, int source2Length = -1);

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