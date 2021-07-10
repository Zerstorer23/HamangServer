#include "IOCP_Server.h"
#include "NetworkMessage.h"
#include "PlayerManager.h"
#include "Player.h"
#include "BufferedMessages.h"
#include "PingManager.h"
#include "MessageHandler.h"
#include "HashTable.h"
IOCP_Server* IOCP_Server::serverInstance = NULL;
MessageHandler IOCP_Server::messageHandler;
IOCP_Server::IOCP_Server() 
{
    hCompletionPort = {};
    serverAddress = {};
    sysInfo = {};
    wsaData = {};
    serverSocket = {};
    shared_ptr<HashTable>  cp(new HashTable());
    customProperty = cp;
};

void IOCP_Server::OpenServer()
{
    //1. CP오브젝트 생성. 마지막인자 0 -> 코어의 수만큼 cp오브젝트에 스레드를 할당
    hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);//마지막인자만 중요
    GetSystemInfo(&sysInfo);
    unsigned int numThread = sysInfo.dwNumberOfProcessors;
    for (unsigned int i = 0; i < numThread; i++) {
        //코어 수만큼 스레드 생성
        _beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hCompletionPort, 0, NULL);
        cout << "Created thread " << i << endl;
    }
    serverSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    SetSocketSize(serverSocket);
    SetSocketReusable(serverSocket);
    BindAddress(serverAddress, 
        ipAddress.c_str()
        , portNumber.c_str());
    int res = bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
    assert(res != SOCKET_ERROR);
    res = listen(serverSocket, 5);
    assert(res != SOCKET_ERROR);

   //1. 뭉쳐 보내기
    //2.

    //receive buffer 1024 /600 , 600 ->첫 600만받고
    //buffer 1024
    LPPER_HANDLE_DATA handleInfo = nullptr;
    int receivedBytes, flags = 0;
    while (true) {
        SOCKET clientSocket;
        SOCKADDR_IN clientAddress;
        int addressLength = sizeof(clientAddress);
        //클라이언트 소켓 생성
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &addressLength);
        SetSocketSize(clientSocket);
        handleInfo = new PER_HANDLE_DATA();
        handleInfo->clientSocket = clientSocket;
        memcpy(&(handleInfo->clientAddress), &clientAddress, addressLength);

        //2. CP오브젝트, 소켓 연결
        //소켓에 overlappedio가 완료되면 cp로 정보 송신, 
        CreateIoCompletionPort((HANDLE)clientSocket, hCompletionPort, (ULONG_PTR)handleInfo, 0); 
        ///->이 단계에서 소켓 리스트를 만들 필요가 있음
        HandlePlayerJoin(handleInfo,clientAddress);

      
        //IO Read이벤트 생성후 구독
        LPPER_IO_DATA ioInfo = CreateEmptyBuffer(BUFFER,READ);
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
        //&(ioInfo->overlapped -> GetQueue...의 4번째  ioInfo로 들어감
    }
    return;
}
unsigned WINAPI IOCP_Server::EchoThreadMain(LPVOID pCompletionPort) {
    HANDLE hCompletionPort = (HANDLE)pCompletionPort;
    DWORD bytesReceived;
    DWORD flags = 0;
    SOCKET clientSocket;
    LPPER_HANDLE_DATA handleInfo;
    LPPER_IO_DATA receivedIO;
    while (true) {
        GetQueuedCompletionStatus(hCompletionPort, &bytesReceived, (PULONG_PTR)&handleInfo, (LPOVERLAPPED*)&receivedIO, INFINITE);
        Player * sourcePlayer = handleInfo->player;
       // cout << "Received " << receivedIO->buffer << endl;
        clientSocket = handleInfo->clientSocket;
        if (receivedIO->rwMode == READ) {
            if (bytesReceived == 0) {
                cout << "들어온 데이터 없음. 종료" << endl;
                HandlePlayerDisconnect(receivedIO,handleInfo, sourcePlayer);
                continue;
            }
            
            string message;

           // receivedIO->buffer = receivedIO->buffer - sourcePlayer->lastBufferPointer;
          //  bytesReceived += sourcePlayer->lastBufferPointer;
            if (bytesReceived < receivedIO->wsaBuf.len) {
                receivedIO->wsaBuf.buf[bytesReceived] = 0;
            }
            message = receivedIO->buffer;
            auto last = receivedIO->wsaBuf.buf[bytesReceived-1];
            bool endWithDelim = last == u8'#';
            /*
             모든 패킷 끝은 #. 이게 없으면 잘린거
             sadf,3,4,45 45455
            */
            
          //  cout << "Received [" << message.length()<< u8"] Message :"s << message << endl;// << " from " << sourcePlayer->actorNumber
            /*
            * TODO cpp SSMS . ODBC
            // Connecting_with_SQLConnect.cpp  
// compile with: user32.lib odbc32.lib  
#include <windows.h>  
#include <sqlext.h>  
#include <mbstring.h>  
#include <stdio.h>  
  
#define MAX_DATA 100  
#define MYSQLSUCCESS(rc) ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO) )  
  
class direxec {  
   RETCODE rc; // ODBC return code  
   HENV henv; // Environment     
   HDBC hdbc; // Connection handle  
   HSTMT hstmt; // Statement handle  
  
   unsigned char szData[MAX_DATA]; // Returned data storage  
   SDWORD cbData; // Output length of data  
   unsigned char chr_ds_name[SQL_MAX_DSN_LENGTH]; // Data source name  
  
public:  
   direxec(); // Constructor  
   void sqlconn(); // Allocate env, stat, and conn  
   void sqlexec(unsigned char *); // Execute SQL statement  
   void sqldisconn(); // Free pointers to env, stat, conn, and disconnect  
   void error_out(); // Displays errors  
};  
  
// Constructor initializes the string chr_ds_name with the data source name.  
// "Northwind" is an ODBC data source (odbcad32.exe) name whose default is the Northwind database  
direxec::direxec() {  
   _mbscpy_s(chr_ds_name, SQL_MAX_DSN_LENGTH, (const unsigned char *)"Northwind");  
}  
  
// Allocate environment handle and connection handle, connect to data source, and allocate statement handle.  
void direxec::sqlconn() {  
   SQLAllocEnv(&henv);  
   SQLAllocConnect(henv, &hdbc);  
   rc = SQLConnect(hdbc, chr_ds_name, SQL_NTS, NULL, 0, NULL, 0);  
  
   // Deallocate handles, display error message, and exit.  
   if (!MYSQLSUCCESS(rc)) {  
      SQLFreeConnect(henv);  
      SQLFreeEnv(henv);  
      SQLFreeConnect(hdbc);  
      if (hstmt)  
         error_out();  
      exit(-1);  
   }  
  
   rc = SQLAllocStmt(hdbc, &hstmt);  
}  
  
// Execute SQL command with SQLExecDirect() ODBC API.  
void direxec::sqlexec(unsigned char * cmdstr) {  
   rc = SQLExecDirect(hstmt, cmdstr, SQL_NTS);  
   if (!MYSQLSUCCESS(rc)) {  //Error  
      error_out();  
      // Deallocate handles and disconnect.  
      SQLFreeStmt(hstmt,SQL_DROP);  
      SQLDisconnect(hdbc);  
      SQLFreeConnect(hdbc);  
      SQLFreeEnv(henv);  
      exit(-1);  
   }  
   else {  
      for ( rc = SQLFetch(hstmt) ; rc == SQL_SUCCESS ; rc=SQLFetch(hstmt) ) {  
         SQLGetData(hstmt, 1, SQL_C_CHAR, szData, sizeof(szData), &cbData);  
         // In this example, the data is sent to the console; SQLBindCol() could be called to bind   
         // individual rows of data and assign for a rowset.  
         printf("%s\n", (const char *)szData);  
      }  
   }  
}  
  
// Free the statement handle, disconnect, free the connection handle, and free the environment handle.  
void direxec::sqldisconn() {  
   SQLFreeStmt(hstmt,SQL_DROP);  
   SQLDisconnect(hdbc);  
   SQLFreeConnect(hdbc);  
   SQLFreeEnv(henv);  
}  
  
// Display error message in a message box that has an OK button.  
void direxec::error_out() {  
   unsigned char szSQLSTATE[10];  
   SDWORD nErr;  
   unsigned char msg[SQL_MAX_MESSAGE_LENGTH + 1];  
   SWORD cbmsg;  
  
   while (SQLError(0, 0, hstmt, szSQLSTATE, &nErr, msg, sizeof(msg), &cbmsg) == SQL_SUCCESS) {  
      sprintf_s((char *)szData, sizeof(szData), "Error:\nSQLSTATE=%s, Native error=%ld, msg='%s'", szSQLSTATE, nErr, msg);  
      MessageBox(NULL, (const char *)szData, "ODBC Error", MB_OK);  
   }  
}  
  
int main () {  
   direxec x;   // Declare an instance of the direxec object.  
   x.sqlconn();   // Allocate handles, and connect.  
   x.sqlexec((UCHAR FAR *)"SELECT FirstName, LastName FROM employees");   // Execute SQL command  
   x.sqldisconn();   // Free handles and disconnect  
}  
            
            */
            //2. split하고
            NetworkMessage netMessage;
            netMessage.Split(message, '#');
            messageHandler.HandleMessage(netMessage);
            if (netMessage.HasMessageToBroadcast()) {
                string msg = netMessage.BuildCopiedMessage();
                PlayerManager::GetInst()->BroadcastMessage(sourcePlayer->actorNumber, msg);
             //   PlayerManager::GetInst()->BroadcastMessageAll(msg);
            }
            int startPoint = netMessage.incompleteResumePoint;
            int length = 0;
            if (startPoint > 0 || !endWithDelim) {
                length = BUFFER - startPoint;
                memcpy(receivedIO->buffer, receivedIO->buffer + startPoint, length);
            }
            IOCP_Server::RecycleIO(receivedIO, READ, length);
            int res = WSARecv(clientSocket, &(receivedIO->wsaBuf), 1, NULL, &flags, &(receivedIO->overlapped), NULL);
            if (res != 0) {
                int cause = WSAGetLastError();
                if (cause != WSA_IO_PENDING) {
                    cout << "Cause " << cause << endl; //https://www.joinc.co.kr/w/man/4100/WSARecv
                    HandlePlayerDisconnect(receivedIO, handleInfo, sourcePlayer);
                }
            }
        }
        else {
            SAFE_DELETE(receivedIO->buffer)
            SAFE_DELETE(receivedIO)
        }
    }
    return 0;

}

void IOCP_Server::HandlePlayerJoin(LPPER_HANDLE_DATA handleInfo, SOCKADDR_IN& clientAddress) {
    // 1.플레이어 추가
    char ipname[128];
    inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
    printf("Connected client IP : %s \n", ipname);
    Player* player = PlayerManager::GetInst()->CreatePlayer(handleInfo);
    PlayerManager::GetInst()->PrintPlayers();
    //2. 룸정보 추가
    NetworkMessage netMessage;
    netMessage.Append(-1);
    netMessage.Append(((int)MessageInfo::ServerCallbacks));
    netMessage.Append(((int)LexCallback::RoomInformationReceived));
    customProperty->EncodeToNetwork(netMessage);
    //3. 다른플레이어 정보 추가
    PlayerManager::GetInst()->EncodePlayersToNetwork(player, netMessage);
    //message = message.append(playerManager.EncodePlayersToNetwork(player));
    
    //4. 서버 현재시간 추가
    long long time = PingManager::GetInst()->GetTimeNow();
    PingManager::GetInst()->RecordPing(player->actorNumber, time);
    netMessage.Append(to_string(time));

    //5. 전송
 /*   wstring message = netMessage.BuildNewSignedMessage();
    LPPER_IO_DATA roomInformationPacket = CreateMessage(message);
    player->Send(roomInformationPacket);*/
     auto message = netMessage.BuildNewSignedMessage();
    // cout<<"Sent to client : "<< message<<endl;
     player->Send(message, true);
     
     //6. RPC
     BufferedMessages::GetInst()->SendBufferedMessages(player);
     NetworkMessage eolMessage;
     eolMessage.Append((player->actorNumber));
     eolMessage.Append(((int)MessageInfo::ServerCallbacks));
     eolMessage.Append(((int)LexCallback::OnLocalPlayerJoined));
     message = eolMessage.BuildNewSignedMessage();
     // LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
     player->Send(message, false);
     cout << "Sent buffered RPCs"<<message.length() << endl;

     //7 .Notify others
     NetworkMessage broadcastMessage;
     broadcastMessage.Append(-1);
     broadcastMessage.Append(((int)MessageInfo::ServerCallbacks));
     broadcastMessage.Append(((int)LexCallback::PlayerJoined));
     player->EncodeToNetwork(broadcastMessage);
     auto brmsg = broadcastMessage.BuildNewSignedMessage();
     PlayerManager::GetInst()->BroadcastMessage(player->actorNumber, brmsg);
}

void IOCP_Server::HandlePlayerDisconnect(LPPER_IO_DATA receivedIO, LPPER_HANDLE_DATA handleInfo, Player * sourcePlayer)
{
    PlayerManager* playerManager = PlayerManager::GetInst();
    auto bufferedMessages = BufferedMessages::GetInst();
    int disconnPlayer = sourcePlayer->actorNumber;
    bool isMaster = sourcePlayer->isMasterClient;
    SAFE_DELETE(handleInfo)
    SAFE_DELETE(receivedIO->buffer)
    SAFE_DELETE(receivedIO)
    playerManager->RemovePlayer(sourcePlayer->actorNumber);

    //actorID , MessageInfo , callbackType, params
    NetworkMessage netMessage;
    netMessage.Append(0);
    netMessage.Append((int)MessageInfo::ServerCallbacks);
    netMessage.Append((int)LexCallback::PlayerDisconnected);
    netMessage.Append(disconnPlayer);
    auto msg = netMessage.BuildNewSignedMessage();
    playerManager->BroadcastMessageAll(msg);

    if (playerManager->GetPlayerCount() == 0) {
        GetInst()->ResetServer();
    }
    else {
        if (isMaster) {
            playerManager->ChangeMasterClientOnDisconnect();
        }
        bufferedMessages->RemovePlayerNr(disconnPlayer);
    } 
    
    cout << "Disconnect Client "<< disconnPlayer<< " / remain "<< playerManager->GetPlayerCount() << endl;
}



void IOCP_Server::ResetServer() {
    BufferedMessages::GetInst()->RemoveAll();
    customProperty->RemoveAllProperties();
    PlayerManager::GetInst()->Reset();
    cout << "Server Reset." << endl;
}