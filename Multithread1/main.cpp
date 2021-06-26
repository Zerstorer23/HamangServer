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
    //constexpr char cp_utf16le[] = ".1200"; // UTF-16 little-endian locale.
  //  setlocale(LC_ALL, cp_utf16le);
  //  _setmode(_fileno(stdout), _O_U16TEXT);
    wcout.imbue(locale("korean"));
    SetConsoleOutputCP(949);
    string ip = "127.0.0.1";
    string port = "9000";
    IOCP_Server * myServer =IOCP_Server::GetInst();
    PlayerManager* playerManager = PlayerManager::GetInst();
    PingManager* pingManager = PingManager::GetInst();
   // pingManager->TimeCheck();
    wcout << L"WCOUT" << endl;
    BufferedMessages* messageBuffer = BufferedMessages::GetInst();
    myServer->InitialiseServer(ip, port);
    myServer->OpenServer();
    myServer->CloseServer();
    IOCP_Server::DestroyInst();
}