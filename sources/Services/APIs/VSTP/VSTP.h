#ifndef VSTP_H
#define VSTP_H

#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <utils.h>

#include "../ApiMediatorInterface.h"
//#include "SocketInfo.h"
#include <logger.h>
#include <TCPServer.h>

#include <utils.h>>
 
//#define MSG_DONTWAIT 0x40

#ifdef __TESTING__
    #include <tester.h>
#endif


//VSTP == VAR SERVER TEXT PROTOCOL
namespace API {
    using namespace std;
    using namespace TCPServerLib;

    class VSTP_ACTIONS{
    public:
        static string SEND_SERVER_INFO_AND_CONFS;
        static string PING;
        static string PONG;
        static string INFORM_CLI_ID;
        static string CHANGE_CLI_ID;
        static string SET_VAR;
        static string GET_VAR;
        static string GET_VAR_RESPONSE;
        static string OBSERVE_VAR;
        static string STOP_OBSERVER_VAR;
        static string VAR_CHANGED;
        static string GET_CHILDS;
        static string GET_CHILDS_RESPONSE;
        static string LOCK_VAR;
        static string UNLOCK_VAR;
        static string LOCK_VAR_DONE;
        static string UNLOCK_VAR_DONE;
    };
    #define VSTP_PROTOCOL_VERSION "1"
    
    //#define VSTP_SCAPE_CHAR '\'

    class VSTP: public ApiInterface
    {    
        private:
            #ifdef __TESTING__
                public:
            #endif

            ApiMediatorInterface *ctrl;
            //ILogger *log;
            NLogger* log;
            TCPServer *server;

            //TODO: use mutex to prevent conflict with clientsById
            map<string, ClientInfo*> clientsById;
            //TODO: use mutex to prevent conflict with incomingDataBuffers
            map<ClientInfo*, string> incomingDataBuffers;

            char scape_char = 0x1B;
            string apiId = "VSTPAPI";

            void initServer(int port);

            void onClientConnected(ClientInfo *cli);
            void onClientDisconnected(ClientInfo *cli);

            void updateClientsByIdList(ClientInfo* cli, string newId = "");
            void sendInfoAndConfToClient(ClientInfo* cli);
            void sendIdToClient(ClientInfo* cli, string id);

            void onDataReceived(ClientInfo* cli, char* data, size_t size);
            bool detectAndTakeACompleteMessage(string &text, string &output);
            void processReceivedMessage(ClientInfo* cli, string message);

            void separateKeyAndValue(string keyValuePair, string &key, string & value, char charSep = '=');
            
            /** 
             * @brief this function is called majority by the function ThreadTalkWithClientFunction when a pack is receitved
             * @param data the received data
             * @param the amount of data (number of bytes) received
             * @param clientSocket a SocketInfo object eith information about the socket client
             */
            void processCommand(string command, string payload, ClientInfo& clientSocket);
            //a helper function to __protocol_VSTP_write
            void __PROTOCOL_VSTP_WRITE(ClientInfo& clientSocket, string command, string data);

            /** 
             * @brief Mounts a VSTP protocol pack and send it to a client socket
             * @param clientSocket A SocketInfo with the scoket client ifnormation who will recieve the data
             * @param command The VSTP command. Take a look in the protocol documentation
             * @param data The arguments of the 'command'. Again: See the protocol documentation
             * @param size The size of 'data'
             * 
            */
            void __PROTOCOL_VSTP_WRITE(ClientInfo& clientSocket, string command, char* data, unsigned int size);
            void ThreadAwaitClientsFunction();
            void ThreadTalkWithClientFunction(int socketClient);

            string byteEscape(string originalText);
            string byteUnescape(string text);


        public:

            VSTP(int port, ApiMediatorInterface *ctr, ILogger *log);
            virtual ~VSTP();

        public:
        /* ApiInterface */
            string getApiId();
            ClientSendResult notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues);
            ClientSendResult checkAlive(string clientId);
    };

    bool SetSocketBlockingEnabled(int fd, bool blocking);
    //void addStringToCharList(vector<char> *destination, string *source, char*source2, int source2Length = -1);

	enum States {READING_COMMAND, READING_NAME, READING_VALUE, FINISHED};

	enum Protocols{
		P_VSTP = 0x02
	};


   
    
}
#endif