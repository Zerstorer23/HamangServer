// 0506multiThreadServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include "PlayerManager.h"
#include "ServerExample.h"
PlayerManager ServerExample::playerManager;

 void ServerExample::OpenServer()
{
    SOCKET clientSocket;
    SOCKADDR_IN clientAddress;
    int clientAddressSize;
    HANDLE thread;

    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("serverSocket error");
    }
    /*Playermanager*/


    BindAddress(servAddr, ipAddress.c_str(), portNumber.c_str());
    int bindResult = bind(serverSocket, (sockaddr*)&servAddr, sizeof(servAddr));
    assert(bindResult != SOCKET_ERROR);

    int listenResult = listen(serverSocket, 5);
    assert(listenResult != SOCKET_ERROR);

    while (true) {
        clientAddressSize = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);
        //Mutex 설정, critical section시작
        Player *player =  playerManager.CreatePlayer(clientSocket);
        //player
        thread = (HANDLE)_beginthreadex(NULL, 0, ServerExample::HandleClient, (void*)player, 0, NULL);
        char ipname[128];
        inet_ntop(AF_INET,(void *)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
        printf("Connected client IP : %s \n", ipname);
    }


}

  unsigned WINAPI  ServerExample::HandleClient(void* arg) {
      Player* player = ((Player*)arg);
      SOCKET clientSocket = player->clientSocket;

    char message[BUFFER];

    //1. 얼마나 받는지 확인
    //IOCP ->  
    //thread
    //2. 요구한만큼 받기
    while (true) {
        
        //recv가 0d이면 상대 클라이언트가 종료
        int expectedBytes = 0;
        int  receivedBytes = recv(clientSocket, (char*)&expectedBytes, sizeof(expectedBytes), 0);
        cout << "received bytes " << receivedBytes << endl;
        cout << "expected byte " << expectedBytes << endl;
     //   printf("e : %s \n",(char*) expectedBytes);
        if (receivedBytes == 0) {
            break;
        };
        if (receivedBytes < 0) {
            cout << " recv fail " << endl;
            break;
        }
        //[1 --- 0]
        receivedBytes = 0;
        while (receivedBytes < expectedBytes) {
            int received = recv(clientSocket, message, expectedBytes, 0);
            receivedBytes += received;
            if (received == 0)break;
            message[expectedBytes + 1] = 0;
            cout << "received bytes " << receivedBytes <<" / expected"<<expectedBytes<<"/ = "<<message <<endl;
        }
        //3. 수신끝. 에코 전송
        ServerExample::playerManager.BroadcastMessageOthers(player->actorNumber, message, receivedBytes);
    }
 //이거 호출되ㅡㄴ지좀 확인
    ServerExample::playerManager.RemovePlayer(player->actorNumber);
    return 0;

}

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
