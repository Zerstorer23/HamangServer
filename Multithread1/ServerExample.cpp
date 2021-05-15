// 0506multiThreadServer.cpp : �� ���Ͽ��� 'main' �Լ��� ���Ե˴ϴ�. �ű⼭ ���α׷� ������ ���۵ǰ� ����˴ϴ�.
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
        //Mutex ����, critical section����
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

    //1. �󸶳� �޴��� Ȯ��
    //IOCP ->  
    //thread
    //2. �䱸�Ѹ�ŭ �ޱ�
    while (true) {
        
        //recv�� 0d�̸� ��� Ŭ���̾�Ʈ�� ����
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
        //3. ���ų�. ���� ����
        ServerExample::playerManager.BroadcastMessageOthers(player->actorNumber, message, receivedBytes);
    }
 //�̰� ȣ��ǤѤ����� Ȯ��
    ServerExample::playerManager.RemovePlayer(player->actorNumber);
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
