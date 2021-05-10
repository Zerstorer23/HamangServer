#include "ServerExample.h"

int main()
{

    char ip_addr[] = "127.0.0.1";
    char mainPort[] = "9000";
    ServerExample myServer;
    if (myServer.InitialiseServer(ip_addr, mainPort) == -1) {
        cout << "Init error " << endl;
    };
    myServer.OpenServer();


}