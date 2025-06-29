#include "VSTP.test.h"

vector<string> VstpTester::getContexts()
{
    return {"APIs.VSTP"};
}

void VstpTester::run(string context)
{
    if (context == "APIs.VSTP")
    {
        //set var
        //get var
        //observate var
        //stop observing var

        //create id
        //change id
        //observate and reconnect
    }
}

int connectToServer(string server, int port)
{
    if (auto socketHandle = socket(AF_INET, SOCK_STREAM, 0); socketHandle >= 0)
    {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server.c_str(), &serv_addr.sin_addr) > 0) 
        {
            int cli_fd;
            if ((cli_fd = connect(socketHandle, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) >= 0) 
                return cli_fd;
        }
    }

    return 0;
}

