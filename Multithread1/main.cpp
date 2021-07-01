#include "IOCP_Server.h"
#include "PlayerManager.h"
#include "PingManager.h"
#include "BufferedMessages.h"
#include <io.h>
#include <fcntl.h>
#include <io.h>
#include <fcntl.h>
#include <locale.h>

int main()
{
    wcout.imbue(locale("korean"));
    SetConsoleOutputCP(949);
   // cout.imbue(locale("korean"));
    string ip = "172.30.1.27";// "127.0.0.1";
    string port = "9000";
    IOCP_Server * myServer =IOCP_Server::GetInst();
    PlayerManager* playerManager = PlayerManager::GetInst();
    PingManager* pingManager = PingManager::GetInst();
    BufferedMessages* messageBuffer = BufferedMessages::GetInst();
    myServer->InitialiseServer(ip, port);
    myServer->OpenServer();
    myServer->CloseServer();
    IOCP_Server::DestroyInst();
}