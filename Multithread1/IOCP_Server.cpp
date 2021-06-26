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
    //1. CP������Ʈ ����. ���������� 0 -> �ھ��� ����ŭ cp������Ʈ�� �����带 �Ҵ�
    hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);//���������ڸ� �߿�
    GetSystemInfo(&sysInfo);
    for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; i++) {
        //�ھ� ����ŭ ������ ����
        _beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hCompletionPort, 0, NULL);
        cout << "Created thread " << i << endl;
    }
    serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    SetSocketSize(serverSocket);
    SetSocketReusable(serverSocket);
    BindAddress(serverAddress, ipAddress.c_str(), portNumber.c_str());
    int res = bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
    assert(res != SOCKET_ERROR);
    res = listen(serverSocket, 5);
    assert(res != SOCKET_ERROR);

   //1. ���� ������
    //2.

    //receive buffer 1024 /600 , 600 ->ù 600���ް�
    //buffer 1024
    LPPER_HANDLE_DATA handleInfo = nullptr;
    int receivedBytes, flags = 0;
    while (true) {
        SOCKET clientSocket;
        SOCKADDR_IN clientAddress;
        int addressLength = sizeof(clientAddress);
        //Ŭ���̾�Ʈ ���� ����
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &addressLength);
        SetSocketSize(clientSocket);
        handleInfo = new PER_HANDLE_DATA();
        handleInfo->clientSocket = clientSocket;
        memcpy(&(handleInfo->clientAddress), &clientAddress, addressLength);

        //2. CP������Ʈ, ���� ����
        //���Ͽ� overlappedio�� �Ϸ�Ǹ� cp�� ���� �۽�, 
        CreateIoCompletionPort((HANDLE)clientSocket, hCompletionPort, (ULONG_PTR)handleInfo, 0); 
        ///->�� �ܰ迡�� ���� ����Ʈ�� ���� �ʿ䰡 ����
        HandlePlayerJoin(handleInfo,clientAddress);

      
        //IO Read�̺�Ʈ ������ ����
        LPPER_IO_DATA ioInfo = CreateEmptyBuffer(BUFFER,READ);
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
        //&(ioInfo->overlapped -> GetQueue...�� 4��°  ioInfo�� ��
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
     //   cout << "Bytes received " << bytesReceived << endl;
        clientSocket = handleInfo->clientSocket;
        if (receivedIO->rwMode == READ) {
            if (bytesReceived == 0) {//����
                HandlePlayerDisconnect(receivedIO,handleInfo, sourcePlayer);
                continue;
            }

            //1. Contents
            string utf8message = receivedIO->buffer;
            wstring message;
            DWORD err = NetworkMessage::convert_utf8_to_unicode_string(message, utf8message.c_str(), utf8message.length());
          //  cout << "ERR " << err << endl;
        //    cout << "Received " << utf8message << " length " << utf8message.length() << "size " << utf8message.size() << endl;
            wcout <<L" Message :" << message << endl;
        /*    for (int i = 0; i < message.size(); i++) {
                cout << "utf8 " << (int)utf8message[i] << " vs unic " << (int)message[i] << endl;
            }*/
            
            //2. split�ϰ�
            NetworkMessage netMessage;
            netMessage.Split(message, '#');
            messageHandler.HandleMessage(netMessage);
            if (netMessage.HasMessageToBroadcast()) {
                wstring msg = netMessage.BuildCopiedMessage();
                PlayerManager::GetInst()->BroadcastMessage(sourcePlayer->actorNumber, msg);
             //   PlayerManager::GetInst()->BroadcastMessageAll(msg);
            }

            // �ٽ� �б���� ��Ȱ��
            IOCP_Server::RecycleIO(receivedIO,READ);
            WSARecv(clientSocket, &(receivedIO->wsaBuf), 1, NULL, &flags, &(receivedIO->overlapped), NULL);
        }
        else {
            cout << "Message sent" << endl;
            cout << " " << endl;
            SAFE_DELETE(receivedIO->buffer)
            SAFE_DELETE(receivedIO)
        }
    }
    return 0;

}

void IOCP_Server::HandlePlayerJoin(LPPER_HANDLE_DATA handleInfo, SOCKADDR_IN& clientAddress) {
    // 1.�÷��̾� �߰�
    char ipname[128];
    inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
    printf("Connected client IP : %s \n", ipname);
    Player* player = PlayerManager::GetInst()->CreatePlayer(handleInfo);
    PlayerManager::GetInst()->PrintPlayers();
    //2. ������ �߰�
    NetworkMessage netMessage;
    netMessage.Append(L"-1");
    netMessage.Append(to_wstring((int)MessageInfo::ServerCallbacks));
    netMessage.Append(to_wstring((int)LexCallback::RoomInformationReceived));
    customProperty->EncodeToNetwork(netMessage);
    //3. �ٸ��÷��̾� ���� �߰�
    PlayerManager::GetInst()->EncodePlayersToNetwork(player, netMessage);
    //message = message.append(playerManager.EncodePlayersToNetwork(player));
    
    //4. ���� ����ð� �߰�
    long long time = PingManager::GetInst()->GetTimeNow();
    PingManager::GetInst()->RecordPing(player->actorNumber, time);
    netMessage.Append(to_wstring(time));

    //5. ����
 /*   wstring message = netMessage.BuildNewSignedMessage();
    LPPER_IO_DATA roomInformationPacket = CreateMessage(message);
    player->Send(roomInformationPacket);*/
     wstring message = netMessage.BuildNewSignedMessage();
    player->Send(message);

}

void IOCP_Server::HandlePlayerDisconnect(LPPER_IO_DATA receivedIO, LPPER_HANDLE_DATA handleInfo, Player * sourcePlayer)
{
    PlayerManager* playerManager = PlayerManager::GetInst();
    int disconnPlayer = sourcePlayer->actorNumber;
    bool isMaster = sourcePlayer->isMasterClient;
    SAFE_DELETE(handleInfo)
    SAFE_DELETE(receivedIO->buffer)
    SAFE_DELETE(receivedIO)
    playerManager->RemovePlayer(sourcePlayer->actorNumber);
    //actorID , MessageInfo , callbackType, params
    NetworkMessage netMessage;
    netMessage.Append(L"0");
    netMessage.Append((int)MessageInfo::ServerCallbacks);
    netMessage.Append((int)LexCallback::PlayerDisconnected);
    netMessage.Append(disconnPlayer);
    wstring msg = netMessage.BuildNewSignedMessage();
    playerManager->BroadcastMessageAll(msg);

    if (playerManager->GetPlayerCount() == 0) {
        BufferedMessages::GetInst()->RemoveAll();
        GetInst()->customProperty->RemoveAllProperties();
    }else if (isMaster) {
        playerManager->ChangeMasterClientOnDisconnect();
    }
    cout << "Disconnect Client "<< disconnPlayer<< " / remain "<< playerManager->GetPlayerCount() << endl;
}



void IOCP_Server::ResetServer() {
    BufferedMessages::GetInst()->RemoveAll();
    customProperty->RemoveAllProperties();
}