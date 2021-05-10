// 0506multiThreadClient.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//


#include <iostream>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <process.h>
#include <cassert>

using namespace std;

void OpenClient(char* ip_addr, char* port);
void BindAddress(SOCKADDR_IN& servAddr, char* ip_addr, char* port) {
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr, &(servAddr.sin_addr.s_addr));
    servAddr.sin_port = htons(atoi(port));
}

#define BUFFER 100
#define NAME_SIZE 20

unsigned WINAPI SendMsg(void* arg);
unsigned WINAPI RecvMsg(void* arg);

char msg[BUFFER];

int main()
{
    WSADATA wsaData;
    char ip_addr[] = "127.0.0.1";
    char mainPort[] = "9000";

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Start up error");
    }
    OpenClient(ip_addr, mainPort);



    WSACleanup();
}

void OpenClient(char* ip_addr, char* port)
{
    SOCKET clientSocket;
    SOCKADDR_IN servAddr;
    HANDLE sendThread, receiveThread;
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    BindAddress(servAddr, ip_addr, port);

    if (connect(clientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        return ;
    }
    sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&clientSocket, 0, NULL);
    receiveThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&clientSocket, 0, NULL);

    WaitForSingleObject(sendThread, INFINITE); //원래 쓰레드 join
    WaitForSingleObject(receiveThread, INFINITE);
    closesocket(clientSocket);

}

unsigned WINAPI SendMsg(void* arg) {
    SOCKET clientSocket = *((SOCKET*)arg);
    while (true) {
        std::cin >> msg;
        if (!strcmp(msg, "q")) {
            closesocket(clientSocket) ;
            exit(0);
        }
        int size = strlen(msg);
        printf("sent: %s %d\n", msg, strlen(msg));
        printf("expected send bytes: %d\n", strlen(msg));

        send(clientSocket, (char*)&size, sizeof(size), 0);
        send(clientSocket, msg, strlen(msg), 0);
    
    }

    return 0;

}
unsigned WINAPI RecvMsg(void* arg) {

    SOCKET clientSocket = *((SOCKET*)arg);
   
    int strlen;
    while (true) {
        int expectedBytes = 0;
        int  receivedBytes = recv(clientSocket, (char*)&expectedBytes, sizeof(expectedBytes), 0);
        cout << "received bytes " << receivedBytes << endl;
        cout << "expected byte " << expectedBytes << endl;
        if (receivedBytes == 0) {
            break;
        };
        receivedBytes = 0;
        while (receivedBytes < expectedBytes) {
            int received = recv(clientSocket, msg, expectedBytes, 0);
            receivedBytes += received;
            if (received == 0)break;
        }
        msg[expectedBytes + 1] = 0;
        cout << "ECHO:"<< msg << endl;
        //3. 수신끝. 에코 전송
    }

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
