#include "IOCP_Server.h"

int main()
{

    string ip = "127.0.0.1";
    string port = "9000";
    IOCP_Server myServer;
    myServer.InitialiseServer(ip, port);
    myServer.OpenServer();
    myServer.CloseServer();
}