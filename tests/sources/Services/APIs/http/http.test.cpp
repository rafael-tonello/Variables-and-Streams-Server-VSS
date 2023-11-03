#include  "http.test.h" 
 
Http_test::Http_test() 
{ 
     
} 
 
Http_test::~Http_test() 
{ 
     
} 
 
/* Tester implementation */
vector<string> Http_test::getContexts()
{
    return { string("APIs.Http") };
}

void Http_test::run(string context)
{
    if (context != string("APIs.Http"))
        return;

    API::HTTP::HttpAPI api();
    
    //TODO: Http api should return valid 'get requests' with a 200 status
    //TODO: Http api should return a 201 no content when setting a var
    //TODO: HTTP API shluld return a forbiden code (401) when try to setting flags and '*' vars
    //TODO: HTTP api should return a fobiden code (401) when try to get a var without sending its name

}


HttpResponse Http_test::request(HttpRequest request)
{
    //int socket = createSocket();

    //doRequest(request, socket);
    //return readResponse(socket);
}


string Http_test::readSocket(int socket)
{

}

void Http_test::writeSocket(int socket, string data)
{
    send(socket, data.c_str(), data.size(), 0);
}

void Http_test::doRequest(HttpRequest request, int socket)
{
    writeSocket(socket, request.method + " ");
    writeSocket(socket, request.url + " HTTP/1.1");
    for (auto &currHeader: request.headers)
        writeSocket(socket, currHeader.exportToHttpHeaderFormat() + "\r\n");

    writeSocket(socket, "\r\n");

    writeSocket(socket, request.body.content);
}



void Http_test::processReadByte(string byte, HttpResponse& out)
{
    if (out.tags["state"] == "reading http version")
    {
        if (byte == " ")
        {
            out.tags["state"] = "reading status code";
            out.tags["buffer"] = "";
        }
    }
    else if (out.tags["state"] == "reading status code")
    {
        if (byte == "")
        {
            out.tags["state"] = "reading http message";
            out.status.code = stoi(out.tags["buffer"]);
            out.tags["buffer"] = "";
        }
        else
            out.tags["buffer"] += byte;
    }
    else if (out.tags["state"] == "reading http message")
    {
        if (byte == "\n")
        {
            out.tags["state"] = "reading next header";
            out.status.message = out.tags["buffer"];
            out.tags["buffer"] = "";
        }
        else if (byte != "\r")
            out.tags["buffer"] += byte;
    }
    else if (out.tags["state"] == "reading http message")
    {
        if (byte == "\n")
        {
            out.tags["state"] = "reading next header";
            out.status.message = out.tags["buffer"];
            out.tags["buffer"] = "";
        }
        else if (byte != "\r")
            out.tags["buffer"] += byte;
    }
    else if (out.tags["state"] == "reading next header")
    {
        if (byte == "\n")
        {
            if (out.tags["buffer"].size() > 2)
            {
                out.headers.push_back(Header::createFromHeaderText(out.tags["buffer"]));
                if (out.headers.back().key == "content-length")
                    out.body.contentLength = atoi(out.headers.back().getValuesAsString().c_str());
                else if (out.headers.back().key == "content-type")
                    out.body.contentType = out.headers.back().getValuesAsString();
                
                out.tags["buffer"] = "";
            }
            else 
            {
                if (out.tags["content-length"] != "")
                {
                    out.tags["state"] = "reading body";
                    out.body.clear();;
                }
                else
                    out.tags["state"] = "finished";
            }
        }
        else if (byte != "\r")
            out.tags["buffer"] += byte;

    }
    else if (out.tags["state"] == "reading body")
    {
        out.body.content += byte;

        if (out.body.content.size() == out.body.contentLength)
            out.tags["state"] = "finished";
    }

}


HttpResponse Http_test::readResponse(int socket)
{
    HttpResponse result;
    result.tags["state"] = "reading http version";
    result.tags["buffer"] = "";

    auto startTime = Utils::getCurrentTimeMilliseconds();
    char buffer[10];

    while (result.tags["state"] != "finished")
    {
        if ((Utils::getCurrentTimeMilliseconds() - startTime) < 2000)
            throw string("timeout");

        auto totalRead = read (socket, buffer, sizeof buffer);

        for (int c = 0; c < totalRead; c++)
            processReadByte(string(1, buffer[c]), result);

    }

    return result;
}
