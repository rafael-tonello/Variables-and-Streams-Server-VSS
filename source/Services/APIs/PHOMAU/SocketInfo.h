#include <time.h>
#include <stdlib.h>

namespace API {
    using namespace std;

    class SocketInfo
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