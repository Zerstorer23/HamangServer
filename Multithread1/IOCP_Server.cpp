#include "IOCP_Server.h"
#include "PlayerManager.h"
PlayerManager IOCP_Server::playerManager;
void IOCP_Server::OpenServer()
{
    //1. CP������Ʈ ����. ���������� 0 -> �ھ��� ����ŭ cp������Ʈ�� �����带 �Ҵ�
    hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);//���������ڸ� �߿�
    GetSystemInfo(&sysInfo);
    for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; i++) {
        //�ھ� ����ŭ ������ ����
        _beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hCompletionPort, 0, NULL);
        cout << "Crated thread " << i << endl;
    }
    serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    BindAddress(serverAddress, ipAddress.c_str(), portNumber.c_str());
    int res = bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
    assert(res != SOCKET_ERROR);
    res = listen(serverSocket, 5);
    assert(res != SOCKET_ERROR);


    //
    LPPER_HANDLE_DATA handleInfo = nullptr;
    int receivedBytes, flags = 0;
    while (true) {
        SOCKET clientSocket;
        SOCKADDR_IN clientAddress;
        int addressLength = sizeof(clientAddress);
        //Ŭ���̾�Ʈ ���� ����
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &addressLength);
        handleInfo = new PER_HANDLE_DATA();
        handleInfo->clientSocket = clientSocket;
        memcpy(&(handleInfo->clientAddress), &clientAddress, addressLength);

        //2. CP������Ʈ, ���� ����
        //���Ͽ� overlappedio�� �Ϸ�Ǹ� cp�� ���� �۽�, 
        CreateIoCompletionPort((HANDLE)clientSocket, hCompletionPort, (ULONG_PTR)handleInfo, 0); //���� DWORD���� struct�ø��ϱ� LONG�޶���
        //client�� ���� io�� �Ϸ�Ǹ� ������ hCompletionPort�� ��ϵ�
        //handleInfo�� ���� �Ű����� ->getQueuedCopletionStatus�� ������ ���ڷ� ���޵�

        ///->�� �ܰ迡�� ���� ����Ʈ�� ���� �ʿ䰡 ����
        char ipname[128];
        inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
        printf("Connected client IP : %s \n", ipname);
        Player* player = playerManager.CreatePlayer(handleInfo);
      
        //IO Read�̺�Ʈ ������ ����
        LPPER_IO_DATA ioInfo = CreateBufferData(BUFFER,READ);
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL); //? �̰� ������?
        //&(ioInfo->overlapped -> GetQueue...�� 4��°  ioInfo�� ��
    }
    return;
}

unsigned WINAPI IOCP_Server::EchoThreadMain(LPVOID pCompletionPort) {
    HANDLE hCompletionPort = (HANDLE)pCompletionPort;

    DWORD bytesReceived;
    // LPPER_IO_DATA headerInfo = new PER_IO_DATA();
    DWORD flags = 0;

    SOCKET clientSocket;
    LPPER_HANDLE_DATA handleInfo;
    LPPER_IO_DATA receivedIO;
    while (true) {
        GetQueuedCompletionStatus(hCompletionPort, &bytesReceived, (PULONG_PTR)&handleInfo, (LPOVERLAPPED*)&receivedIO, INFINITE);
        //WSARecv�� ������ 

        Player * sourcePlayer = handleInfo->player;

        clientSocket = handleInfo->clientSocket;
        if (receivedIO->rwMode == READ) {
            if (bytesReceived == 0) {//����
                cout << "Disconnect Client" << endl;
                SAFE_DELETE(handleInfo)
                SAFE_DELETE(receivedIO)
                playerManager.RemovePlayer(sourcePlayer->actorNumber);
                //Callback": Disconnect �۽�
                continue;
            }
            //1. Contents
            cout << "Received Message :" << receivedIO->buffer << endl;
            //ServerRequest�� �� �ܰ迡�� �۽�
            //C#���� �����ϱ� ù��°�� �̻��� ĳ���� ����
                    //1. ù�� �����
            string message = receivedIO->buffer;
            message.erase(0,1);

            cout << "Cleansed Message :" << message << endl;
            //2. split�ϰ�
            vector<string> tokens = Split(message, '#');
            for (unsigned int i = 0; i < tokens.size(); i++) {
                cout << tokens[i] << endl;
            }
            if (tokens.size() >= 2) {
                //3. 0 1 <- 1��° �ڵ� int�� �а� aoi
                int messageInfo = stoi(tokens[1]);
                if (messageInfo == MSG_REQUEST) {
                    //4. request�� 2��° �ڵ� �а� switch
                    int messageCode = stoi(tokens[2]);

                }
                else if (messageInfo == MSG_HASH) {
                    //SetHash�� ���� Hash�� ������Ʈ �ʿ�
                    //actorNum, SetHash [int]roomOrPlayer [string]Key [object]value
                }
                else {
                    //�׿ܿ��� BroadCast
                    playerManager.BroadcastMessage(sourcePlayer->actorNumber, receivedIO, bytesReceived);
                }
            
            }
            else {
                playerManager.BroadcastMessage(sourcePlayer->actorNumber, receivedIO, bytesReceived);
            }
       

     


            // �ٽ� �б���� ��Ȱ��
            memset(&(receivedIO->overlapped), 0, sizeof(OVERLAPPED));
            receivedIO->wsaBuf.len = BUFFER;
            receivedIO->wsaBuf.buf = receivedIO->buffer;
            receivedIO->rwMode = READ;
            WSARecv(clientSocket, &(receivedIO->wsaBuf), 1, NULL, &flags, &(receivedIO->overlapped), NULL);


        }
        else {
            cout << "Message sent" << endl;
            SAFE_DELETE(receivedIO)
        }
    }
    return 0;

}
