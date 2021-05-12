// 0506multiThreadClient.cpp : �� ���Ͽ��� 'main' �Լ��� ���Ե˴ϴ�. �ű⼭ ���α׷� ������ ���۵ǰ� ����˴ϴ�.
//


#include <iostream>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <process.h>
#include <cassert>
#include <string>


using namespace std;

void OpenClient(char* ip_addr, char* port);
void BindAddress(SOCKADDR_IN& servAddr, char* ip_addr, char* port) {
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr, &(servAddr.sin_addr.s_addr));
    servAddr.sin_port = htons(atoi(port));
}

#define BUFFER 1024
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
        return;
    }
    sendThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&clientSocket, 0, NULL);
    receiveThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&clientSocket, 0, NULL);

    WaitForSingleObject(sendThread, INFINITE); //���� ������ join
    WaitForSingleObject(receiveThread, INFINITE);
    closesocket(clientSocket);

}

unsigned WINAPI SendMsg(void* arg) {
    SOCKET clientSocket = *((SOCKET*)arg);
    while (true) {
        string line;
        getline(cin, line);
        strcpy_s(msg, line.c_str());
        if (!strcmp(msg, "q")) {
            closesocket(clientSocket);
            return 0;
        }
        int size = strlen(msg);
        printf("sent: %s %d\n", msg, strlen(msg));
        // printf("expected send bytes: %d\n", strlen(msg));

       //  send(clientSocket, (char*)&size, sizeof(size), 0);
        send(clientSocket, msg, strlen(msg), 0);

    }

    return 0;

}
unsigned WINAPI RecvMsg(void* arg) {

    SOCKET clientSocket = *((SOCKET*)arg);

    while (true) {
        int received = recv(clientSocket, msg, BUFFER, 0);
        if (received == 0) break;
        msg[received] = 0;
        cout << "ECHO:" << msg << endl;
    }

    return 0;
}
// ���α׷� ����: <Ctrl+F5> �Ǵ� [�����] > [��������� �ʰ� ����] �޴�
// ���α׷� �����: <F5> Ű �Ǵ� [�����] > [����� ����] �޴�

// ������ ���� ��: 
//   1. [�ַ�� Ž����] â�� ����Ͽ� ������ �߰�/�����մϴ�.
//   2. [�� Ž����] â�� ����Ͽ� �ҽ� ��� �����մϴ�.
//   3. [���] â�� ����Ͽ� ���� ��� �� ��Ÿ �޽����� Ȯ���մϴ�.
//   4. [���� ���] â�� ����Ͽ� ������ ���ϴ�.
//   5. [������Ʈ] > [�� �׸� �߰�]�� �̵��Ͽ� �� �ڵ� ������ ����ų�, [������Ʈ] > [���� �׸� �߰�]�� �̵��Ͽ� ���� �ڵ� ������ ������Ʈ�� �߰��մϴ�.
//   6. ���߿� �� ������Ʈ�� �ٽ� ������ [����] > [����] > [������Ʈ]�� �̵��ϰ� .sln ������ �����մϴ�.
