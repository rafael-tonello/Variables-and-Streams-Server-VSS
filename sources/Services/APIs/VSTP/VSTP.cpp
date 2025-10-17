 #include "VSTP.h"

string API::VSTP_ACTIONS::SEND_SERVER_INFO_AND_CONFS = "serverinfo";
string API::VSTP_ACTIONS::PING = "ping";
string API::VSTP_ACTIONS::PONG = "pong";
string API::VSTP_ACTIONS::SUGGEST_NEW_CLI_ID = "sugestednewid";
string API::VSTP_ACTIONS::CHANGE_OR_CONFIRM_CLI_ID = "setid";
string API::VSTP_ACTIONS::TOTAL_VARIABLES_ALREADY_BEING_OBSERVED = "aoc";
string API::VSTP_ACTIONS::RESPONSE_BEGIN = "beginresponse";
string API::VSTP_ACTIONS::RESPONSE_END = "endresponse";
string API::VSTP_ACTIONS::SET_VAR = "set";
string API::VSTP_ACTIONS::DELETE_VAR = "delete";
string API::VSTP_ACTIONS::DELETE_VAR_RESULT = "deleteresult";
string API::VSTP_ACTIONS::GET_VAR = "get";
string API::VSTP_ACTIONS::GET_VAR_RESPONSE = "varvalue";
string API::VSTP_ACTIONS::SUBSCRIBE_VAR = "subscribe";
string API::VSTP_ACTIONS::UNSUBSCRIBE_VAR = "unsubscribe";
string API::VSTP_ACTIONS::VAR_CHANGED = "varchanged";
string API::VSTP_ACTIONS::GET_CHILDS = "getchilds";
string API::VSTP_ACTIONS::GET_CHILDS_RESPONSE = "childs";
string API::VSTP_ACTIONS::LOCK_VAR = "lock";
string API::VSTP_ACTIONS::UNLOCK_VAR = "unlock";
string API::VSTP_ACTIONS::LOCK_VAR_RESULT = "lockresult";
string API::VSTP_ACTIONS::UNLOCK_VAR_DONE = "unlockdone";
string API::VSTP_ACTIONS::SERVER_BEGIN_HEADERS = "beginserverheaders";
string API::VSTP_ACTIONS::SERVER_END_HEADERS = "endserverheaders";
string API::VSTP_ACTIONS::HELP = "help";
string API::VSTP_ACTIONS::SET_TELNET_SESSION = "telnet";
string API::VSTP_ACTIONS::CHECK_VAR_LOCK_STATUS = "lockstatus";
string API::VSTP_ACTIONS::CHECK_VAR_LOCK_STATUS_RESULT = "lockstatusresult";
string API::VSTP_ACTIONS::ERROR = "error";

API::VSTP::VSTP(int port, DependencyInjectionManager &dim)
{
    this->log =   dim.get<ILogger>()->getNamedLoggerP("API::VSTP");
    this->ctrl = dim.get<ApiMediatorInterface>();
    this->scheduler = dim.get<ThreadPool>();
    this->SecondaryScheduler = dim.get<ThreadPool>("ThreadPool2");
    this->initServer(port, dim.get<ThreadPool>());
    this->startListenMessageBus(dim.get<MessageBus<JsonMaker::JSON>>());

    //this->log.log(LOGGER_LOGLEVEL_INFO2, (DVV){"VSTP service started with port: ", port});

    ctrl->apiStarted(this);
}

API::VSTP::~VSTP()
{
    delete this->server;
}

void API::VSTP::VSTP::initServer(int port, ThreadPool *tasker)
{
    this->port = port;

    this->server = new TCPServer();
    auto startResult = this->server->startListen({
        shared_ptr<TCPServer_SocketInputConf>(new TCPServer_PortConf(port))
    });

    if (startResult.startedPorts.size() > 0)
    {
        this->server->addConEventListener([&](shared_ptr<ClientInfo> client, CONN_EVENT event){
            if (event == CONN_EVENT::CONNECTED)
                this->onClientConnected(client);
            else
                this->onClientDisconnected(client);
        });

        this->server->addReceiveListener([&](shared_ptr<ClientInfo> client, char* data,  size_t size){
            this->onDataReceived(client, data, size);
        });

        this->log->info("VSTP API is running and listen at the port "+to_string(port));
    }
    else
    {
        for (auto &p: startResult.failedPorts)
        {
            //auto [portinfo, error] = p;
            auto portinfo= std::get<0>(p);
            auto error = std::get<1>(p);

            TCPServer_PortConf *castedPort = (TCPServer_PortConf*)portinfo.get();
            this->log->error("Cannot start the tcp server at port "+to_string(castedPort->port)+": " + error + ". VSTP API service is not running.");
        }

        this->port = -1;
        return;
    }
}

void API::VSTP::VSTP::onClientConnected(shared_ptr<ClientInfo>  cli)
{
    //create an unique id and sent id to updateClientAboutObservatingVarsthe client
    cli->tags["id"] = Utils::createUniqueId();

    sendBeginHeaderToClient(cli);



    sendInfoAndConfToClient(cli);
    updateClientsByIdList(cli, cli->tags["id"]);
    incomingDataBuffers[cli] = "";
    
    sendIdToClient(cli, cli->tags["id"]);
    //sentTotalVarsAlreadyBeingObserved(cli, 0);

    TCPServer_PortConf *portConf = (TCPServer_PortConf*)&cli->inputSocketInfo;
    
    // check if cli->inputSocketInfo is a TCPServer_PortConf
    
    log->info((DVV){"Client", cli->address, "connected and received the id ","'"+cli->tags["id"]+"'","and friendly name", "'"+getCliFriendlyName(cli) + "'"});
    

    sendEndHeaderToClient(cli);
    
    //NOTE:Do not notify controller about the new client id, becasuse it can be only a temporary connection or
    //client can ignore this id and send a new one. 
    //Controller will automatically register it when it observates a variable
    //
    //Threrefore, when client send its id, is very importante that controller be notiyed about its reconnection, to update it about observating vars.

}

void API::VSTP::VSTP::onClientDisconnected(shared_ptr<ClientInfo>  cli)
{
    log->info((DVV){"Client", getCliFriendlyName(cli, true), "disconnected"});
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

void API::VSTP::VSTP::sendBeginHeaderToClient(shared_ptr<ClientInfo>  cli)
{
    __PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::SERVER_BEGIN_HEADERS, "");
}

void API::VSTP::VSTP::sendEndHeaderToClient(shared_ptr<ClientInfo>  cli)
{
    __PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::SERVER_END_HEADERS, "");
}

void API::VSTP::VSTP::sendInfoAndConfToClient(shared_ptr<ClientInfo>  cli)
{
    __PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::SEND_SERVER_INFO_AND_CONFS, string("PROTOCOL VERSION=")+string(VSTP_PROTOCOL_VERSION));
    __PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::SEND_SERVER_INFO_AND_CONFS, string("VSS VERSION=")+ctrl->getSystemVersion());


    //scapeChar must be sent without __PROTOCOL_VSTP_WRITE
    string buffer = VSTP_ACTIONS::SEND_SERVER_INFO_AND_CONFS + CMDPAYLOADSEPARATOR + "SCAPE CHARACTER="+scape_char + "\n";
    cli->sendString(buffer);
}

void API::VSTP::VSTP::sendIdToClient(shared_ptr<ClientInfo>  cli, string id)
{
    __PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::SUGGEST_NEW_CLI_ID, id);
}

void API::VSTP::VSTP::sentTotalVarsAlreadyBeingObserved(shared_ptr<ClientInfo> cli, int varsCount)
{
    __PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::TOTAL_VARIABLES_ALREADY_BEING_OBSERVED, to_string(varsCount));
}

void API::VSTP::VSTP::sendErrorToClient(shared_ptr<ClientInfo> cli, Errors::Error error)
{
    log->warning("Sending error to client '"+getCliFriendlyName(cli)+"': "+error);
    //log->info2("Sending error to client '"+getCliFriendlyName(cli)+"': "+error.message);
    __PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::ERROR, error);
}

void API::VSTP::VSTP::sendErrorToClient(shared_ptr<ClientInfo> cli, string commandWithError, Errors::Error AdditionalError)
{
    string errorMessage = commandWithError+"; Error processing command '" + commandWithError + "': "+ AdditionalError;
    this->sendErrorToClient(cli, Errors::Error(errorMessage));
}

void API::VSTP::VSTP::updateClientsByIdList(shared_ptr<ClientInfo>  cli, string newId)
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
    clientsById[newId] = cli;
}

void API::VSTP::processCommand(string command, string payload, shared_ptr<ClientInfo> clientSocket)
{
    //if (command != VSTP_ACTIONS::PING)
    //    this->log->debug((DVV){"received command from client "+clientSocket->tags["id"] + " ("+clientSocket->address+"): ", command,  "payload.size():", (int)payload.size(), "payload:", Utils::StringToHex(payload)});
        
    string strData;
    string key;
    string tempStr;
    string varName;
    string value;
    char* buffer;

    future<GetVarResult> get_var_fut;
    VarList get_var_values;

    separateKeyAndValue(payload, varName, value, "=;: ");
    if (command != "" && string(VSTP_ACTIONS::HELP + "\r").find(command) == 0)
        displayHelpMenu(clientSocket);
    else if (command != "" && string(VSTP_ACTIONS::SET_TELNET_SESSION + "\r").find(command) == 0)
    {
        log->info("Session of client '"+getCliFriendlyName(clientSocket, true)+"' setted as a telnet session.");
        clientSocket->tags["isATelnetSession"] = "true";
        clientSocket->sendString("Session configured as a Telenet session.\n");
    }
    else if (command ==VSTP_ACTIONS::SET_VAR)
    {
        auto sv_ctrl_result = this->ctrl->setVar(varName, value).get();

        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        if (sv_ctrl_result != Errors::NoError)
            this->sendErrorToClient(clientSocket, VSTP_ACTIONS::SET_VAR, sv_ctrl_result);
        
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
    }
    else if (command ==VSTP_ACTIONS::DELETE_VAR)
    {
        auto dv_ctrl_result = this->ctrl->delVar(varName).get();

        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        if (dv_ctrl_result != Errors::NoError)
            this->sendErrorToClient(clientSocket, VSTP_ACTIONS::DELETE_VAR, dv_ctrl_result);

        string resultMsg = dv_ctrl_result == Errors::NoError ? "sucess": "failure:"+dv_ctrl_result;
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::DELETE_VAR_RESULT, varName + "=" + resultMsg);
        
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
    }
    else if (command == VSTP_ACTIONS::GET_VAR)
    {
        //store a clientSocket id in a variable
        Utils::named_lock("VSTP::processCommand::GET_VAR::"+clientSocket->tags["id"], [&](){
            get_var_fut = this->ctrl->getVar(varName, DynamicVar(string("")));
            get_var_values = get_var_fut.get().result;
            

            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
            for (auto &c : get_var_values)
            {
                //strin as buffer. I know, this is is not a good praticy.. May be i change this sometime
                string bufferStr = std::get<0>(c) + "="+(std::get<1>(c)).getString();
                this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::GET_VAR_RESPONSE , bufferStr);
            }
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
        }, 2000);
    }
    else if (command == VSTP_ACTIONS::LOCK_VAR)
    {
        //note: with the actual structure and socket system, this operation will block que socket reading until the var is sucessful locked
        uint timeout = UINT_MAX;
        try{
            if (value != "")
                timeout = stoi(value);
        }
        catch (...)
        {
            log->error((DVV){"Error parsing timeout, to LOCKVAR action, received from client", getCliFriendlyName(clientSocket, true) + ".", "Received value was:", payload});
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::LOCK_VAR_RESULT, varName + "=Error parsing timeout");

            return;
        }

        log->debug("Locking var "+varName);
        auto lockFuture = this->ctrl->lockVar(varName, timeout);
        auto result = lockFuture.get();
        string resultMsg = result == Errors::NoError ? "sucess": "failure:"+result;
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::LOCK_VAR_RESULT, varName + "=" + resultMsg);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
        log->debug("Variable "+varName+" is now locked. Response was writed with sucess");
    }
    else if (command == VSTP_ACTIONS::UNLOCK_VAR)
    {
        //note: with the actual structure and socket system, this operation will block que socket reading until the var is sucessful locked
        log->debug("Unlocking var "+varName);
        auto lockFuture = this->ctrl->unlockVar(varName);
        lockFuture.get();
        log->debug("writing unlockvar response");
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::UNLOCK_VAR_DONE, varName);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
        log->debug("Variable "+varName+" is now unlocked. Response was writed with sucess");
    }
    else if (command == VSTP_ACTIONS::CHECK_VAR_LOCK_STATUS)
    {
        auto varLockCheckResult = this->ctrl->isVarLocked(varName);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::CHECK_VAR_LOCK_STATUS_RESULT, varName + "=" + (varLockCheckResult ? "locked" : "unlocked"));
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
    }
    else if (command == VSTP_ACTIONS::SUBSCRIBE_VAR)
    {

        auto [ vName, customIdsAndMetainfo] = separateNameAndMetadata(varName);
        varName = vName;

        ctrl->observeVar(varName, clientSocket->tags["id"], customIdsAndMetainfo, this);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);

        log->info({"Client", clientSocket->tags["id"], "is now observing the variable", varName});
    }
    else if (command == VSTP_ACTIONS::UNSUBSCRIBE_VAR)
    {
        auto [ vName, customIdsAndMetainfo] = separateNameAndMetadata(varName);
        varName = vName;

        //todo: if no metadata is specified, delete all client observations

        ctrl->stopObservingVar(varName, clientSocket->tags["id"], customIdsAndMetainfo, this);
        log->info({"Client", clientSocket->tags["id"], "stop watching the variable", varName});

        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
    }
    else if (command == VSTP_ACTIONS::GET_CHILDS)
    {
        auto resultFromController  = this->ctrl->getChildsOfVar(varName).get();
        if (resultFromController.status == Errors::NoError)
        {
            vector<string> result = resultFromController.result;
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
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::GET_CHILDS_RESPONSE, buffer, response.size());
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);

            //clear used data
            delete[] buffer;
            result.clear();
            response.clear();
        }
        else
        {
            log->error("Error returned from controller when running GET_CHILDS action: "+resultFromController.status);
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
            this->sendErrorToClient(clientSocket, VSTP_ACTIONS::GET_CHILDS, resultFromController.status);
            this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
        }
    }
    else if (command == VSTP_ACTIONS::PING)
    {
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::PONG, "");
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
    }
    else if (command == VSTP_ACTIONS::CHANGE_OR_CONFIRM_CLI_ID)
    {
        string oldId = clientSocket->tags["id"];
        if (oldId != payload)
        {
            this->updateClientsByIdList(clientSocket, payload);
            log->info((DVV){"Client", clientSocket->address, "(address",clientSocket->address,") changed its id from", oldId, "to", payload});
        }
        else
        {
            log->info((DVV){"The client", clientSocket->address, "(address",clientSocket->address,") requested an id change with its actual id (", oldId,")"});
        }

        //even if client do not change its id, notify the controller so it can update the client about its observing vars.
        int varsAlreadyBeingObserved = 0;
        this->ctrl->clientConnected(payload, this, varsAlreadyBeingObserved);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        sentTotalVarsAlreadyBeingObserved(clientSocket, varsAlreadyBeingObserved);
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
    }
    else 
    {
        log->info("Received an unknown command: '"+command+"'. Payload: '"+payload+"'");
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_BEGIN, command + CMDPAYLOADSEPARATOR + payload);
        sendErrorToClient(clientSocket, command, Errors::Error("Unknown command"));
        this->__PROTOCOL_VSTP_WRITE(clientSocket, VSTP_ACTIONS::RESPONSE_END, command + CMDPAYLOADSEPARATOR + payload);
        if (clientSocket->tags["isATelnetSession"] == "true")
        {
            clientSocket->sendString("Unknown command '"+command+". Use 'help' command to get more information about commands.'\n");
        }
    }

    //if (command != VSTP_ACTIONS::PING)
    //    this->log->debug((DVV){"processCommand is exiting. Command:", command,  "payload.size():", (int)payload.size(), "payload:", Utils::StringToHex(payload)});
}

void API::VSTP::__PROTOCOL_VSTP_WRITE(shared_ptr<ClientInfo> clientSocket, string command, string data)
{
    //if (data.size() > 0)
    //    data = byteEscape(data);

    string buffer = command + CMDPAYLOADSEPARATOR + data + "\n";

    this->log->debug("sending '"+buffer+"' to client "+clientSocket->tags["id"] + " ("+clientSocket->address+")");
    clientSocket->sendString(buffer);
}

void API::VSTP::__PROTOCOL_VSTP_WRITE(shared_ptr<ClientInfo> clientSocket, string command, char* data, unsigned int size)
{
    __PROTOCOL_VSTP_WRITE(clientSocket, command, string(data, size));
}

string API::VSTP::byteEscape(string originalText)
{
    return Utils::sr(originalText, {
        {"\n", string(1, scape_char) + string("n")},
        {string(1, scape_char), string(1, scape_char+scape_char)}
    });
}

string API::VSTP::byteUnescape(string text)
{

    return Utils::sr(text, {
        {string(1, scape_char) + string("n"), "\n"},
        {string(1, scape_char)+string(1, scape_char), string(1, scape_char)}
    });
}

void API::VSTP::onDataReceived(shared_ptr<ClientInfo>  cli, char* data, size_t size)
{
    if (incomingDataBuffers.count(cli) == 0)
    {
        log->error("Received data from an unknown client. Ignoring this data.");
        return;
    }
    
    incomingDataBuffers[cli] += string(data, size);
    string package;

    while (detectAndTakeACompleteMessage(incomingDataBuffers[cli], package, cli->tags["isATelnetSession"] == "true"))
        processReceivedMessage(cli, package);
}

bool API::VSTP::detectAndTakeACompleteMessage(string &text, string &output, bool isATelnetSession)
{
    if (size_t pos = text.find('\n'); pos != string::npos)
    {
        output = text.substr(0, pos);
        text = text.substr(pos+1);

        if (isATelnetSession && output.size() > 0 && output[output.size()-1] == '\r')
            output = output.substr(0, output.size()-1);

        return true;
    }
    return false;
}

void API::VSTP::processReceivedMessage(shared_ptr<ClientInfo>  cli, string message)
{
    //log->info2("::processReceivedMessage("+to_string((uint64_t)cli)+", \""+message+"\")");
    string command = "";
    string payload = "";

    separateKeyAndValue(message, command, payload, ";: ");

    //payload = byteUnescape(payload);

    
    //use the secondary scheduler because this tasks needs to wait for futures that depends on primary scheduler and, as primary scheduler,
    //has a limited number of threads, it can run out in a deadlock;
    SecondaryScheduler->enqueue([&, command, payload, cli](){
        this->processCommand(command, payload, cli);
    });
}

void API::VSTP::separateKeyAndValue(string keyValuePair, string &key, string & value, string possibleCharSeps)
{   
    for (size_t c = 0; c < keyValuePair.size(); c++)
    {
        if (possibleCharSeps.find(keyValuePair[c]) != string::npos)
        {
            key = keyValuePair.substr(0, c);
            value = keyValuePair.substr(c+1);    
            return;
        }
    }

    key = keyValuePair;
    value = "";
}

void API::VSTP::separateKeyAndValue(string keyValuePair, string &key, string & value, char charSep)
{
    string sep = "";
    sep += charSep;
    return separateKeyAndValue(keyValuePair, key, value, sep);
}

tuple<string, string> API::VSTP::separateNameAndMetadata(string originalVarName)
{
    if (auto pos = originalVarName.find('('); pos != string::npos)
    {
        auto varname = originalVarName.substr(0, pos);
        auto metadata = originalVarName.substr(pos + 1);
        if (metadata.size() > 0 && metadata[metadata.size()-1] == ')')
            metadata = metadata.substr(0, metadata.size()-1);

        return { varname, metadata };
    }
    else
        return {originalVarName, ""};
    
}


string API::VSTP::getApiId()
{
    return this->apiId;
}


API::ClientSendResult API::VSTP::notifyClient(string clientId, vector<tuple<string, string, DynamicVar>> varsnamesMetadataAndValues)
{
    if (clientsById.count(clientId))
    {
        auto cli = clientsById[clientId];
        for (auto &c: varsnamesMetadataAndValues)
        {
            string metadataStr = std::get<1>(c) != "" ? "("+std::get<1>(c)+")": "";
            string bufferStr = std::get<0>(c) + metadataStr+ "="+(std::get<2>(c)).getString();
            this->__PROTOCOL_VSTP_WRITE(cli, VSTP_ACTIONS::VAR_CHANGED, bufferStr);
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

string API::VSTP::getListeningInfo()
{
    if (this->port > -1)
        return "TCP/"+to_string(port);
    else
        return "Error - No opened port";
}

void API::VSTP::startListenMessageBus(MessageBus<JsonMaker::JSON> *bus)
{
    bus->listen("discover.startedApis", [&](string message, JsonMaker::JSON args, JsonMaker::JSON &result){
        if (this->port > -1)
        {
            result.setString("name", "VSTP - Var stream protocol");
            result.setString("access", getListeningInfo());
        }
    });
}

string API::VSTP::getCliFriendlyName(shared_ptr<ClientInfo>  cli, bool includeClieIdAndAditionalInfomation)
{
    if (!cli->tags.count("friendlyName"))
        cli->tags["friendlyName"] = Utils::getAName(rand()) + " " + Utils::getAName(rand());

    string ret = cli->tags["friendlyName"];

    if (includeClieIdAndAditionalInfomation)
    ret += " (ID: "+cli->tags["id"]+", address: "+cli->address+")";

    return ret;
}

void API::VSTP::displayHelpMenu(shared_ptr<ClientInfo>  cli)
{
    cli->sendString(string("VSTP rotocol version ") + string(VSTP_PROTOCOL_VERSION) + string("\n"));
    cli->sendString(string("\n"));
    cli->sendString(string("    Usage <command> [arguments]\n"));
    cli->sendString(string("    Usage (2, using space instead ';'): vsp_command arguments\n"));
    cli->sendString(string("\n"));
    //                      --------------------------------------------------------------------------------
    cli->sendString(string("    Commands: \n"));
    cli->sendString(string("        help - Display this menu\n"));
    cli->sendString(string("        set <varname>=<value> - Changes or create the variable 'varname' with value 'value';\n"));
    cli->sendString(string("        get <varname>         - Gets the value of variable 'varname'. Wildcard(* char) can be used here;\n"));
    cli->sendString(string("        lock <varname>        - Locks the variable 'varname' ('sv' and others commands will not be able to change the variable);\n"));
    cli->sendString(string("        u nlock <varname>      - Unlocks the variable 'varname';\n"));
    cli->sendString(string("        lockstatus <varname>  - Returns 'locked' (if 'varname' is locked) or 'unlocked' (if 'varname' is unlocked);\n"));
    cli->sendString(string("        subscribe <varname>   - Subscribe the variable 'varname'. The server will send a message to the client when the variable changes;\n"));
    cli->sendString(string("        unsubscribe <varname> - Cancels the subscription to the variable 'varname';\n"));
    cli->sendString(string("        getchilds <varname>   - Get variable 'varname' childs. Note: The system uses object name notation and uses the dot (.) as name separator;\n"));
    cli->sendString(string("        ping                  - Pings the server. Server will replay with 'pong;';\n"));
    cli->sendString(string("        setid                 - Update the client id. It is used to resume a previous VSTP session. The server will reply with all observed variables and theirs respective values;\n"));
    cli->sendString(string("        telnet                - Informs the server that the current session is a telnet session. Server will adapt messages to enable a more easy use over a telnet session (like remove \\' from the end of received commands)\n"));
    cli->sendString(string("\n"));
}