#ifndef _SOCKET_INFO_H
#define _SOCKET_INFO_H
#include <time.h>
#include <stdlib.h>
#include "../../../Shared/Misc/TaggedObject.h"

namespace API{
    using namespace std;

    class SocketInfo: public TaggedObject 
    {
        private:
            long id;

        public:
            int socket;
            time_t lastKeepAlive;
            SocketInfo();
            long getId();
            void upadateAlive();
            int getAliveSeconds();
    };
}

#endif