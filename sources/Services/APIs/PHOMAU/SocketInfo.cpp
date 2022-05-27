#include "SocketInfo.h"
namespace API {
    SocketInfo::SocketInfo()
    {
        this->upadateAlive();
        this->id = 0; //not defined yet
    }

    long SocketInfo::getId()
    {
        if (this->id == 0)
        {
            (int)lastKeepAlive;
            this->id += rand();
            this->id += this->socket;
        }

        return this->id;
    }

    void SocketInfo::upadateAlive()
    {
        time(&(this->lastKeepAlive));
    }

    int SocketInfo::getAliveSeconds()
    {
        time_t currTime;
        time(&currTime);

        double secsD = difftime(currTime, this->lastKeepAlive);
        int ret = (int)secsD;

        return ret;
    }
}