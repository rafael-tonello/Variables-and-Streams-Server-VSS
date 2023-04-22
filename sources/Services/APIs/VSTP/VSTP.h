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

#include <utils.h>
#include <dependencyInjectionManager.h>
#include <messagebus.h>
#include <JSON.h>
 
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
        static string SUGGEST_NEW_CLI_ID;
        static string CHANGE_OR_CONFIRM_CLI_ID;
        static string TOTAL_VARIABLES_ALREADY_BEING_OBSERVED;
        static string SET_VAR;
        static string GET_VAR;
        static string GET_VAR_RESPONSE;
        static string SUBSCRIBE_VAR;
        static string UNSUBSCRIBE_VAR;
        static string VAR_CHANGED;
        static string GET_CHILDS;
        static string GET_CHILDS_RESPONSE;
        static string LOCK_VAR;
        static string UNLOCK_VAR;
        static string LOCK_VAR_DONE;
        static string UNLOCK_VAR_DONE;
        static string SERVER_BEGIN_HEADERS;
        static string SERVER_END_HEADERS;
        static string HELP;
        static string SET_TELNET_SESSION;
    };
    #define VSTP_PROTOCOL_VERSION "1.1.0"
    
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
            int port;

            //TODO: use mutex to prevent conflict with clientsById
            map<string, ClientInfo*> clientsById;
            //TODO: use mutex to prevent conflict with incomingDataBuffers
            map<ClientInfo*, string> incomingDataBuffers;

            char scape_char = 0x1B;
            string apiId = "VSTPAPI";

            void initServer(int port, ThreadPool *tasker);

            void onClientConnected(ClientInfo *cli);
            void onClientDisconnected(ClientInfo *cli);

            void updateClientsByIdList(ClientInfo* cli, string newId = "");
            void sendBeginHeaderToClient(ClientInfo* cli);
            void sendEndHeaderToClient(ClientInfo* cli);
            void sendInfoAndConfToClient(ClientInfo* cli);
            void sendIdToClient(ClientInfo* cli, string id);
            void sentTotalVarsAlreadyBeingObserved(ClientInfo *cli, int varCount);

            void onDataReceived(ClientInfo* cli, char* data, size_t size);
            bool detectAndTakeACompleteMessage(string &text, string &output, bool isATelnetSession = false);
            void processReceivedMessage(ClientInfo* cli, string message);

            void displayHelpMenu(ClientInfo* cli);

            string getCliFriendlyName(ClientInfo* cli, bool includeClieIdAndAditionalInfomation = false);

            /**
             * @brief Separate key and value from a keyValuePair. The funciton will find the first ocurrency of any character in 'possibleCharSeps' arguments
             * and will divide the keyValuePair in this position
             * 
             * @param keyValuePair A string with a key, followed by a separator and by a value.
             * @param key Key output
             * @param value Value output
             * @param possibleCharSeps Possible characters to be used as a key-value seperator.
             */
            void separateKeyAndValue(string keyValuePair, string &key, string & value, string possibleCharSeps);


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

            void startListenMessageBus(MessageBus<JsonMaker::JSON> *bus);


        public:

            VSTP(int port, DependencyInjectionManager &dim);
            virtual ~VSTP();
            string getListeningInfo();

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