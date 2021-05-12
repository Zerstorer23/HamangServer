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
        CreateIoCompletionPort((HANDLE)clientSocket, hCompletionPort, (ULONG_PTR)handleInfo, 0); //���� DWORD����
        //client�� ���� io�� �Ϸ�Ǹ� ������ hCompletionPort�� ��ϵ�
        //handleInfo�� ���� �Ű����� ->getQueuedCopletionStatus�� ������ ���ڷ� ���޵�

        ///->�� �ܰ迡�� ���� ����Ʈ�� ���� �ʿ䰡 ����
        char ipname[128];
        inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
        printf("Connected client IP : %s \n", ipname);
        Player* player = playerManager.CreatePlayer(handleInfo);
        //
        ioInfo = new PER_IO_DATA();
        memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
        ioInfo->wsaBuf.len = BUFFER;
        ioInfo->wsaBuf.buf = ioInfo->buffer;
        ioInfo->rwMode = READ; // IOCP ��ȣ���� ����� ������ ��� ���� �־����
        //READ�� ���

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
    LPPER_IO_DATA ioInfo;
    while (true) {
        GetQueuedCompletionStatus(hCompletionPort, &bytesReceived, (PULONG_PTR)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
        //WSARecv�� ������ 

        Player * sourcePlayer = handleInfo->player;

        clientSocket = handleInfo->clientSocket;
        if (ioInfo->rwMode == READ) {
            if (bytesReceived == 0) {//����
                cout << "Disconnect Client" << endl;
                SAFE_DELETE(handleInfo)
                SAFE_DELETE(ioInfo)
                playerManager.RemovePlayer(sourcePlayer->actorNumber);
                continue;
            }
            cout << "Received Message :" << ioInfo->buffer << endl;
            //1. Contents

            memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
            ioInfo->wsaBuf.len = bytesReceived;
            ioInfo->rwMode = WRITE;

            //LPPER_IO_DATA targetIO = new PER_IO_DATA();
            //memset(&(targetIO->overlapped), 0, sizeof(OVERLAPPED));
            //targetIO->wsaBuf.len = bytesReceived;
            //targetIO->wsaBuf.buf = ioInfo->buffer;
            //strcpy_s(targetIO->buffer, ioInfo->buffer);
            //targetIO->rwMode = WRITE;
            //�۽Ÿ��
            //--> �� �ܰ迡�� ��� ���Ͽ� ���� �ʿ䰡 ����
            for (auto entry : playerManager.playerHash) {
                Player* targetPlayer = entry.second;
                if (targetPlayer->actorNumber == sourcePlayer->actorNumber) continue;
                SOCKET targetSocket = targetPlayer->handleInfo->clientSocket;
                // LPPER_IO_DATA target_ioInfo = entry.second->;
                cout << "Actor number " << entry.second->actorNumber << endl;
                //2.Contents //Broadcast�ȵ�
               // send(targetSocket, (ioInfo->wsaBuf.buf), (ioInfo->wsaBuf.len), 0);
                WSASend(targetSocket, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
            }


            // �ٽ� �б���

            ioInfo = new PER_IO_DATA();
            memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
            ioInfo->wsaBuf.len = BUFFER;
            ioInfo->wsaBuf.buf = ioInfo->buffer;
            ioInfo->rwMode = READ;
            WSARecv(clientSocket, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);


        }
        else {
            cout << "Message sent" << endl;
            /* if (ioInfo != nullptr) {
                 delete ioInfo; //TODO �̰� ����� �ȵǴµ�
                 ioInfo = nullptr; //ioInfo ���� ������ ���ؼ� �˾ƺ�����
             }*/
        }
    }
    return 0;

}
