#include "VSTP.h"

string API::VSTP_ACTIONS::SEND_SERVER_INFO_AND_CONFS = "sci";
string API::VSTP_ACTIONS::PING = "ping";
string API::VSTP_ACTIONS::PONG = "pong";
string API::VSTP_ACTIONS::SUGGEST_NEW_CLI_ID = "nid";
string API::VSTP_ACTIONS::CHANGE_OR_CONFIRM_CLI_ID = "cid";
string API::VSTP_ACTIONS::SET_VAR = "sv";
string API::VSTP_ACTIONS::GET_VAR = "gv";
string API::VSTP_ACTIONS::GET_VAR_RESPONSE = "gvr";
string API::VSTP_ACTIONS::OBSERVE_VAR = "ov";
string API::VSTP_ACTIONS::STOP_OBSERVER_VAR = "sov";
string API::VSTP_ACTIONS::VAR_CHANGED = "vc";
string API::VSTP_ACTIONS::GET_CHILDS = "gc";
string API::VSTP_ACTIONS::GET_CHILDS_RESPONSE = "gcr";
string API::VSTP_ACTIONS::LOCK_VAR = "lv";
string API::VSTP_ACTIONS::UNLOCK_VAR = "uv";
string API::VSTP_ACTIONS::LOCK_VAR_DONE = "lvd";
string API::VSTP_ACTIONS::UNLOCK_VAR_DONE = "uvd";

API::VSTP::VSTP(int port, DependencyInjectionManager &dim)
{
    this->log =   dim.get<ILogger>()->getNamedLoggerP("API::VSTP");
    this->ctrl = dim.get<ApiMediatorInterface>();
    this->initServer(port, dim.get<ThreadPool>());

    //this->log.log(LOGGER_LOGLEVEL_INFO2, (DVV){"VSTP service started with port: ", port});

    ctrl->apiStarted(this);
}

API::VSTP::~VSTP()
{
    delete this->server;
}

void API::VSTP::VSTP::initServer(int port, ThreadPool *tasker)
{
    bool sucess;
    this->port = port;

    this->server = new TCPServer(port, sucess, tasker);
    this->server->addConEventListener([&](ClientInfo *client, CONN_EVENT event){
        if (event == CONN_EVENT::CONNECTED)
            this->onClientConnected(client);
        else
            this->onClientDisconnected(client);
    });

    if (sucess)
    {
        this->server->addConEventListener([&](ClientInfo *client, CONN_EVENT event){
            if (event == CONN_EVENT::CONNECTED)
                this->onClientConnected(client);
            else
                this->onClientDisconnected(client);
        });

        this->server->addReceiveListener([&](ClientInfo *client, char* data,  size_t size){
            this->onDataReceived(client, data, size);
        });

        this->log->info("VSTP API is running and listen at the port "+to_string(port));
    }
    else
    {
        this->log->error("Cannot start the tcp server at port "+to_string(port)+". VSTP API service is not running");
        this->port = -1;
        return;
    }
}

void API::VSTP::VSTP::onClientConnected(ClientInfo* cli)
{
    //create an unique id and sent id to updateClientAboutObservatingVarsthe client
    cli->tags["id"] = Utils::createUniqueId();
    sendInfoAndConfToClient(cli);
    updateClientsByIdList(cli, cli->tags["id"]);
    incomingDataBuffers[cli] = "";
    
    sendIdToClient(cli, cli->tags["id"]);

    log->info((DVV){"Cient", cli->address, "(remote port:",cli->port,") connected and received the id '",cli->tags["id"],"'"});
    
    //NOTE:Do not notify controller about the new client id, becasuse it can be only a temporary connection or
    //client can ignore this id and send a new one. 
    //Controller will automatically register it when it observates a variable
    //
    //Threrefore, when client send its id, is very importante that controller be notiyed about its reconnection, to update it about observating vars.

}

void API::VSTP::VSTP::onClientDisconnected(ClientInfo* cli)
{
    if (incomingDataBuffers.count(cli))
    {
        incomingDataBuffers[cli] = "";
        incomingDataBuffers.erase(cli);
    }

    if (clientsById.count(cli->tags["id"]))
    {
        clientsById[cli->tags["id"]] = NULL;
        clientsById.erase(cli->tags["id"]);
    }
}

void API::VSTP::VSTP::sendInfoAndConfToClient(ClientInfo* cli)
{
    __PROTOCOL_VSTP_WRITE(*cli, VSTP_ACTIONS::SEND_SERVER_INFO_AND_CONFS, string("PROTOCOL VERSION=")+string(VSTP_PROTOCOL_VERSION));


    //scapeChar must be sent without __PROTOCOL_VSTP_WRITE
    string buffer = VSTP_ACTIONS::SEND_SERVER_INFO_AND_CONFS + ";" + "SCAPE CHARACTER="+scape_char + "\n";
    cli->sendString(buffer);
    buffer.clear();
}

void API::VSTP::VSTP::sendIdToClient(ClientInfo* cli, string id)
{
    __PROTOCOL_VSTP_WRITE(*cli, VSTP_ACTIONS::SUGGEST_NEW_CLI_ID, id);
}


void API::VSTP::VSTP::updateClientsByIdList(ClientInfo* cli, string newId)
{
    if (newId == "")
        newId = cli->tags["id"];

    string oldId = cli->tags["id"];

    if (clientsById.count(oldId))
    {
        clientsById[oldId] = NULL;
        clientsById.erase(oldId);
    }
    
    cli->tags["id"] = newId;
    clientsById[newId]  = cli;
}


void API::VSTP::processCommand(string command, string payload, ClientInfo &clientSocket)
{
    this->log->info((DVV){"processCommand received a new command:", command,  "payload.size():", (int)payload.size(), "payload:", Utils::StringToHex(payload)});
    string strData;
    string key;
    string tempStr;
    string varName;
    string value;
    char* buffer;

    future<vector<tuple<string, DynamicVar>>> get_var_fut;
    vector<tuple<string, DynamicVar>> get_var_values;

    separateKeyAndValue(payload, varName, value);
    if (command ==VSTP_ACTIONS::SET_VAR)
        this->ctrl->setVar(varName, value);
    else if (command == VSTP_ACTIONS::GET_VAR)
    {
        //store a clientSocket id in a variable
        get_var_fut = this->ctrl->getVar(varName, DynamicVar(string("")));
        get_var_values = get_var_fut.get();
        

        for (auto &c : get_var_values)
        {
            //strin as buffer. I know, this is is not a good praticy.. May be i change this sometime
            string bufferStr = std::get<0>(c) + "="+(std::get<1>(c)).getString();
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::GET_VAR_RESPONSE , bufferStr);
        }
    }
    else if (command == VSTP_ACTIONS::LOCK_VAR)
    {
        //note: with the actual structure and socket system, this operation will block que socket reading until the var is sucessful locked
        auto lockFuture = this->ctrl->lockVar(varName);
        lockFuture.get();
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::LOCK_VAR_DONE, varName);
    }
    else if (command == VSTP_ACTIONS::UNLOCK_VAR)
    {
        //note: with the actual structure and socket system, this operation will block que socket reading until the var is sucessful locked
        auto lockFuture = this->ctrl->unlockVar(varName);
        lockFuture.get();
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::UNLOCK_VAR_DONE, varName);
    }
    else if (command == VSTP_ACTIONS::OBSERVE_VAR)
    {
        ctrl->observeVar(varName, clientSocket.tags["id"], this);
        log->info({"Client", clientSocket.tags["id"], "is now observing the variable", varName});
    }
    else if (command == VSTP_ACTIONS::STOP_OBSERVER_VAR)
    {
        ctrl->stopObservingVar(varName, clientSocket.tags["id"], this);
        log->info({"Client", clientSocket.tags["id"], "stop watching the variable", varName});
    }
    else if (command == VSTP_ACTIONS::GET_CHILDS)
    {
        vector<string> result = this->ctrl->getChildsOfVar(varName).get();
        string response = "";
        for (size_t cont = 0; cont < result.size(); cont++)
        {
            response += result[cont];
            if (cont < result.size()-1)
                response += ",";
            
        }

        //prepara buffer with response and send this
        buffer = new char[response.size()];
        for (size_t cont  =0; cont < response.size(); cont++)
            buffer[cont] = response[cont];

        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::GET_CHILDS_RESPONSE, buffer, response.size());

        //clear used data
        delete[] buffer;
        result.clear();
        response.clear();
    }
    else if (command == VSTP_ACTIONS::PING)
    {
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::PONG, "");
    }
    else if (command == VSTP_ACTIONS::CHANGE_OR_CONFIRM_CLI_ID)
    {
        string oldId = clientSocket.tags["id"];
        if (oldId != payload)
        {
            this->updateClientsByIdList(&clientSocket, payload);
            log->info((DVV){"Client", clientSocket.address, "(remote port",clientSocket.port,") changed its id from", oldId, "to", payload});
        }
        else
        {
            log->info((DVV){"The client", clientSocket.address, "(remote port",clientSocket.port,") requested an id change with its actual id (", oldId,")"});
        }

        //event if client do not change its id, notify the controller so it can update the client about its observing vars.
        this->ctrl->clientConnected(payload, this);
    }
}


void API::VSTP::__PROTOCOL_VSTP_WRITE(ClientInfo& clientSocket, string command, string data)
{
    string buffer = command + ";" + byteEscape(data) + "\n";
    clientSocket.sendString(buffer);
    buffer.clear();
}

void API::VSTP::__PROTOCOL_VSTP_WRITE(ClientInfo& clientSocket, string command, char* data, unsigned int size)
{
    __PROTOCOL_VSTP_WRITE(clientSocket, command, string(data, size));
}

string API::VSTP::byteEscape(string originalText)
{
    stringstream ret("");
    for (auto &c: originalText)
    {
        if (c == '\n')
            ret << scape_char << 'n';
        else if (c == scape_char)
            ret << scape_char << scape_char;
        else
            ret << c;
    }

    return ret.str();
}

string API::VSTP::byteUnescape(string text)
{
    log->warning("::byteUnescape is not impelemented yet (return the original text)");
    return text;
}

void API::VSTP::onDataReceived(ClientInfo* cli, char* data, size_t size)
{
    incomingDataBuffers[cli] += string(data, size);
    string package;

    while (detectAndTakeACompleteMessage(incomingDataBuffers[cli], package))
        processReceivedMessage(cli, package);
}

bool API::VSTP::detectAndTakeACompleteMessage(string &text, string &output)
{
    if (size_t pos = text.find('\n'); pos != string::npos)
    {
        output = text.substr(0, pos);
        text = text.substr(pos+1);
        return true;
    }
    return false;
}

void API::VSTP::processReceivedMessage(ClientInfo* cli, string message)
{
    log->info2("::processReceivedMessage("+to_string((uint64_t)cli)+", \""+message+"\")");
    string command = "";
    string payload = "";

    separateKeyAndValue(message, command, payload, ';');

    payload = byteUnescape(payload);

    this->processCommand(command, payload, *cli);
}

void API::VSTP::separateKeyAndValue(string keyValuePair, string &key, string & value, char charSep)
{
    if (auto pos = keyValuePair.find(charSep); pos != string::npos)
    {
        key = keyValuePair.substr(0, pos);
        value = keyValuePair.substr(pos+1);
    }
    else
    {
        key = keyValuePair;
        value = "";
    }
}


string API::VSTP::getApiId()
{
    return this->apiId;
}


API::ClientSendResult API::VSTP::notifyClient(string clientId, vector<tuple<string, DynamicVar>> varsAndValues)
{
    if (clientsById.count(clientId))
    {
        auto cli = clientsById[clientId];
        for (auto &c: varsAndValues)
        {
            string bufferStr = std::get<0>(c) + "="+(std::get<1>(c)).getString();
            this->__PROTOCOL_VSTP_WRITE(*cli, VSTP_ACTIONS::VAR_CHANGED, bufferStr);
        }
        return ClientSendResult::LIVE;
    }
    return ClientSendResult::DISCONNECTED;
}

API::ClientSendResult API::VSTP::checkAlive(string clientId)
{
    if (clientsById.count(clientId))
    {
        auto cli = clientsById[clientId];
        if (cli->isConnected())
            return ClientSendResult::LIVE;
    }
    
    return ClientSendResult::DISCONNECTED;
}

string API::VSTP::getRunningPortInfo()
{
    if (this->port > -1)
        return "TCP/"+to_string(port);
    else
        return "Error - No opened port";
}