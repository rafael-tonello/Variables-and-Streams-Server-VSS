#include "SocketInfo.h"
namespace API {
    SocketInfo::SocketInfo()
    {
        this->upadateKeepAlive();
        this->id = (int)lastKeepAlive;
        this->id += rand();
    }

    long SocketInfo::getId()
    {
        return this->id;
    }

    void SocketInfo::upadateKeepAlive()
    {
        time(&(this->lastKeepAlive));
    }

    int SocketInfo::getKeepAliveSeconds()
    {
        time_t currTime;
        time(&currTime);

        double secsD = difftime(currTime, this->lastKeepAlive);
        int ret = (int)secsD;

        return ret;
    }
}