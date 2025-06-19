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
#include <limits.h>
#include <timersForDebug.h>

//#define MSG_DONTWAIT 0x40

#ifdef __TESTING__
    #include <tester.h>
#endif

#define CMDPAYLOADSEPARATOR  ":"

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
        static string RESPONSE_BEGIN;
        static string RESPONSE_END;
        static string SET_VAR;
        static string DELETE_VAR;
        static string DELETE_VAR_RESULT;
        static string GET_VAR;
        static string GET_VAR_RESPONSE;
        static string SUBSCRIBE_VAR;
        static string UNSUBSCRIBE_VAR;
        static string VAR_CHANGED;
        static string GET_CHILDS;
        static string GET_CHILDS_RESPONSE;
        static string LOCK_VAR;
        static string UNLOCK_VAR;
        static string LOCK_VAR_RESULT;
        static string UNLOCK_VAR_DONE;
        static string CHECK_VAR_LOCK_STATUS;
        static string CHECK_VAR_LOCK_STATUS_RESULT;
        static string SERVER_BEGIN_HEADERS;
        static string SERVER_END_HEADERS;
        static string HELP;
        static string SET_TELNET_SESSION;
        static string ERROR;
    };
    #define VSTP_PROTOCOL_VERSION "1.2.0"
    
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
            map<string, shared_ptr<ClientInfo>> clientsById;
            //TODO: use mutex to prevent conflict with incomingDataBuffers
            map<ClientInfo*, string> incomingDataBuffers;

            char scape_char = 0x1B;
            string apiId = "VSTPAPI";
            
            void initServer(int port, ThreadPool *tasker);

            void onClientConnected(shared_ptr<ClientInfo> cli);
            void onClientDisconnected(shared_ptr<ClientInfo> cli);

            void updateClientsByIdList(shared_ptr<ClientInfo>  cli, string newId = "");
            void sendBeginHeaderToClient(shared_ptr<ClientInfo>  cli);
            void sendEndHeaderToClient(shared_ptr<ClientInfo>  cli);
            void sendInfoAndConfToClient(shared_ptr<ClientInfo>  cli);
            void sendIdToClient(shared_ptr<ClientInfo>  cli, string id);
            void sentTotalVarsAlreadyBeingObserved(shared_ptr<ClientInfo> cli, int varCount);
            void sendErrorToClient(shared_ptr<ClientInfo> cli, Errors::Error error);
            void sendErrorToClient(shared_ptr<ClientInfo> cli, string commandWithError, Errors::Error AdditionalError);

            void onDataReceived(shared_ptr<ClientInfo>  cli, char* data, size_t size);
            bool detectAndTakeACompleteMessage(string &text, string &output, bool isATelnetSession = false);
            void processReceivedMessage(shared_ptr<ClientInfo>  cli, string message);

            void displayHelpMenu(shared_ptr<ClientInfo>  cli);

            string getCliFriendlyName(shared_ptr<ClientInfo>  cli, bool includeClieIdAndAditionalInfomation = false);

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

            /// @brief seperate metada from varname
            /// @param originalVarName the original varname (with the medata)
            /// @return a tuple with varname and metadata --> tuple<string varname, string metadata>
            tuple<string, string> separateNameAndMetadata(string originalVarName);
            
            /** 
             * @brief this function is called majority by the function ThreadTalkWithClientFunction when a pack is receitved
             * @param data the received data
             * @param the amount of data (number of bytes) received
             * @param clientSocket a SocketInfo object eith information about the socket client
             */
            void processCommand(string command, string payload, shared_ptr<ClientInfo> clientSocket);
            //a helper function to __protocol_VSTP_write
            void __PROTOCOL_VSTP_WRITE(shared_ptr<ClientInfo> clientSocket, string command, string data);

            /** 
             * @brief Mounts a VSTP protocol pack and send it to a client socket
             * @param clientSocket A SocketInfo with the scoket client ifnormation who will recieve the data
             * @param command The VSTP command. Take a look in the protocol documentation
             * @param data The arguments of the 'command'. Again: See the protocol documentation
             * @param size The size of 'data'
             * 
            */
            void __PROTOCOL_VSTP_WRITE(shared_ptr<ClientInfo> clientSocket, string command, char* data, unsigned int size);
            void ThreadAwaitClientsFunction();
            void ThreadTalkWithClientFunction(int socketClient);

            string byteEscape(string originalText);
            string byteUnescape(string text);

            void startListenMessageBus(MessageBus<JsonMaker::JSON> *bus);

            ThreadPool *scheduler;
            ThreadPool *SecondaryScheduler;


        public:

            VSTP(int port, DependencyInjectionManager &dim);
            virtual ~VSTP();
            string getListeningInfo();

        public:
        /* ApiInterface */
            string getApiId();
            ClientSendResult notifyClient(string clientId, vector<tuple<string, string, DynamicVar>> varsnamesMetadataAndValues);
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