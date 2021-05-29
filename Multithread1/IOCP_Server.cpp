#include "IOCP_Server.h"
#include "NetworkMessage.h"
#include "PlayerManager.h"
#include "Player.h"
#include "BufferedMessages.h"
#include "PingManager.h"
PlayerManager IOCP_Server::playerManager;
BufferedMessages IOCP_Server::bufferedRPCs;
PingManager IOCP_Server::pingManager;
IOCP_Server* IOCP_Server::serverInstance = NULL;
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
        LPPER_IO_DATA ioInfo = CreateBufferData(BUFFER,READ);
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
        //&(ioInfo->overlapped -> GetQueue...�� 4��°  ioInfo�� ��
    }
    return;
}
void IOCP_Server::HandlePlayerJoin(LPPER_HANDLE_DATA handleInfo, SOCKADDR_IN& clientAddress) {
    // 1.�÷��̾� �߰�
    char ipname[128];
    inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
    printf("Connected client IP : %s \n", ipname);
    Player* player = playerManager.CreatePlayer(handleInfo);
    playerManager.PrintPlayers();
    //2. ������ �߰�
    NetworkMessage netMessage;
    netMessage.Append("-1");
    netMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    netMessage.Append(to_string((int)LexCallback::RoomInformationReceived));
    EncodeServerToNetwork(netMessage);
    //Players
    playerManager.EncodePlayersToNetwork(player, netMessage);
    //message = message.append(playerManager.EncodePlayersToNetwork(player));
    string message = netMessage.BuildNewSignedMessage();
    LPPER_IO_DATA roomInformationPacket = CreateMessage(message);
    player->Send(roomInformationPacket);

    //3 �ð� ����ȭ 1������
    pingManager.Handle_Request_TimeSynch(player,0,1);
}

/*
 1024 < - 600 / 400 200
 
 SETSOCKET... 1mb

 //a 1 2 3
 //  1[2..1..3] 2 3
 
 
 ��ī�� ����  == 
 
 */
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
        //40 30 // 40 10��
        //Shared pointer
        Player * sourcePlayer = handleInfo->player;
        cout << "Bytes received " << bytesReceived << endl;
        clientSocket = handleInfo->clientSocket;
        if (receivedIO->rwMode == READ) {
            if (bytesReceived == 0) {//����
                cout << "Disconnect Client" << endl;
                int disconnPlayer = sourcePlayer->actorNumber;
                SAFE_DELETE(handleInfo)
                SAFE_DELETE(receivedIO->buffer)
                SAFE_DELETE(receivedIO)
                playerManager.RemovePlayer(sourcePlayer->actorNumber);
                HandlePlayerDisconnect(disconnPlayer);
                //Callback": Disconnect �۽�
                continue;
            }
            //1. Contents
           // cout << "Received Message :" << receivedIO->buffer << endl;
            //ServerRequest�� �� �ܰ迡�� �۽�
            //C#���� �����ϱ� ù��°�� �̻��� ĳ���� ����
                    //1. ù�� �����
            string message = receivedIO->buffer;
            //message.erase(0,1);

            cout << "Cleansed Message :" << message << endl;
            //2. split�ϰ�
            NetworkMessage netMessage;
            netMessage.Split(message, '#');
            HandleMessage(netMessage);
            if (netMessage.HasMessageToBroadcast()) {
                string msg = netMessage.BuildCopiedMessage();
               // playerManager.BroadcastMessage(sourcePlayer->actorNumber, sendBuffer, bytesReceived);
                playerManager.BroadcastMessageAll((char *)msg.c_str(), bytesReceived);
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
            cout << " " << endl;
            SAFE_DELETE(receivedIO->buffer)
            SAFE_DELETE(receivedIO)
        }
    }
    return 0;

}
void IOCP_Server::HandlePlayerDisconnect(int disconnectActorID) 
{

    //actorID , MessageInfo , callbackType, params
    NetworkMessage netMessage;
    netMessage.Append("0");
    netMessage.Append((int)MessageInfo::ServerCallbacks);
    netMessage.Append((int)LexCallback::PlayerDisconnected);
    netMessage.Append(disconnectActorID);
    string msg = netMessage.BuildNewSignedMessage();
    DWORD size = msg.length() + 1;
    playerManager.BroadcastMessageAll((char *)msg.c_str(),size);
}


void IOCP_Server::HandleMessage(NetworkMessage & netMessage)
{
    while (netMessage.HasNext()) {
        int beginPointOfAMessage = netMessage.iterator;
        //1. Check signature
        string signature = netMessage.GetNext();
        bool isMyPacket =(   signature.compare(NET_SIG) == 0);
        if (!isMyPacket) continue;

        //2. �޼��� ���� �б�
        int lengthOfMessages = stoi(netMessage.GetNext());
        //3. �޼��� �߾��� �б�
        netMessage.sentActorNr = stoi(netMessage.GetNext());
        //4. �޼��� Ÿ�� �б�
        MessageInfo messageInfo = (MessageInfo)stoi(netMessage.GetNext());
        int endPointOfAMessage = beginPointOfAMessage + lengthOfMessages;
        cout << signature << "Is my packet from "<<beginPointOfAMessage<<" to "<<endPointOfAMessage << endl;
        if (messageInfo == MessageInfo::ServerRequest) {
            cout << "Received server request" << endl;
            //4. request�� 2��° �ڵ� �а� switch
            Handle_ServerRequest(netMessage);
        }
        else if (messageInfo == MessageInfo::SetHash) {
            cout << "Handle hash : " << (int)messageInfo << endl;
            Handle_PropertyRequest(netMessage);
            netMessage.SaveStrings(beginPointOfAMessage, endPointOfAMessage);
        }
        else {
            cout << "Received echo message" << endl;
            //0 /1 2 3 4/ 5 6
            //�Ѵ����� �����
            string message =  netMessage.SaveStrings(beginPointOfAMessage, endPointOfAMessage);//end exclusive
            if (messageInfo == MessageInfo::RPC || messageInfo == MessageInfo::Instantiate 
                || messageInfo == MessageInfo::Destroy || messageInfo == MessageInfo::SyncVar) {
                //��� ��۸޼����� 0 sig / 1 len / 2 sender / 3 type / 4 viewID ����
                //������ �ʿ��� Ÿ�Ե�
                netMessage.targetViewID = stoi(netMessage.GetNext());
                bufferedRPCs.EnqueueMessage(netMessage.sentActorNr, netMessage.targetViewID, message);
            }
            netMessage.iterator = endPointOfAMessage;
        }
    }
   


}

void IOCP_Server::Handle_PropertyRequest(NetworkMessage& netMessage)
{
    //SetHash�� ���� Hash�� ������Ʈ �ʿ�
    //actorNum, SetHash [int]roomOrPlayer [string]Key [object]value
    int target = stoi(netMessage.GetNext());
    cout << "Set hash on " << endl;
    string key = netMessage.GetNext();
    string value = netMessage.GetNext();
    if (target == 0) {
        IOCP_Server::GetInst()->SetProperty(key, value);
        IOCP_Server::GetInst()->PrintProperties();
    }
    else {
        playerManager.playerHash[target]->SetProperty(key, value);
        playerManager.playerHash[target]->PrintProperties();
    }
}
void IOCP_Server::Handle_ServerRequest(NetworkMessage& netMessage)
{
    LexRequest messageCode = (LexRequest)stoi(netMessage.GetNext());
    switch (messageCode)
    {
    case LexRequest::None:
        break;
    case LexRequest::RemoveRPC:
        Handle_ServerRequest_RemoveRPCs(netMessage);
        break;
    case LexRequest::Receive_modifiedTime:
        Handle_ServerRequest_ReceiveModifiedTime(netMessage);
        break;
    case LexRequest::ChangeMasterClient:
        Handle_ServerRequest_ChangeMasterClient(netMessage);
        break;
    }

}

void IOCP_Server::Handle_ServerRequest_ChangeMasterClient(NetworkMessage& netMessage) {
    //actorID , MessageInfo , callbackType, params
    int newMasterActor = stoi(netMessage.GetNext());
    playerManager.SetMasterClient(newMasterActor);
    NetworkMessage eolMessage;
    eolMessage.Append(to_string(netMessage.sentActorNr));
    eolMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_string((int)LexCallback::MasterClientChanged));
    eolMessage.Append(to_string(newMasterActor));
    string message = eolMessage.BuildNewSignedMessage();
    DWORD size = message.length();
    LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    playerManager.BroadcastMessageAll((char*)message.c_str(), size);
}
void IOCP_Server::Handle_ServerRequest_SendBufferedRPCs(Player* target) {
    bufferedRPCs.SendBufferedMessages(target);

    NetworkMessage eolMessage;
    eolMessage.Append(to_string(target->actorNumber));
    eolMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_string((int)LexCallback::OnLocalPlayerJoined));
    string message = eolMessage.BuildNewSignedMessage();
    LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    target->Send(sendIO);
    cout << "Sent buffered RPCs" << endl;

    //actorID , MessageInfo , callbackType, params
// PlayerJoined�� �������� ��ü���
    NetworkMessage broadcastMessage;
    broadcastMessage.Append("-1");
    broadcastMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    broadcastMessage.Append(to_string((int)LexCallback::PlayerJoined));
    target->EncodeToNetwork(broadcastMessage);
    string brmsg = broadcastMessage.BuildNewSignedMessage();
    DWORD size = (DWORD)brmsg.length();
    playerManager.BroadcastMessage(target->actorNumber, (char*)brmsg.c_str(), size);
    cout << "IO Created" << endl;
}

void IOCP_Server::Handle_ServerRequest_RemoveRPCs(NetworkMessage& netMessage) {
    int actorID = stoi(netMessage.GetNext());
    int viewID = stoi(netMessage.GetNext());
    if (actorID != -1) {
        if (viewID != -1) {

            bufferedRPCs.RemoveRPC(actorID, viewID);
        }
        else {
            bufferedRPCs.RemovePlayerNr(actorID);
        }
    }
    else {
            bufferedRPCs.RemoveViewID(viewID);
    }

}
void IOCP_Server::Handle_ServerRequest_ReceiveModifiedTime(NetworkMessage& netMessage)
{
    //LEX / 0 =SERVER / PING=MESSAGEINFO / targetPlater /1 OR 0 = INDEX TO REFER / SERVERTIME or EXPECTEDDELAY //Part of local Join
    int targetPlayerNumber = stoi(netMessage.GetNext());
    int isMod = stoi(netMessage.GetNext());//always 1
    (netMessage.GetNext());
    int isLocal = stoi(netMessage.GetNext());
    Player* target = playerManager.playerHash[targetPlayerNumber];
    pingManager.Handle_Request_TimeSynch(target, isMod, isLocal);
    if (isLocal) {
        Handle_ServerRequest_SendBufferedRPCs(target);
    }
}
;
void IOCP_Server::EncodeServerToNetwork(NetworkMessage& netMessage) {
    netMessage.Append(to_string(serverCustomProperty.size()));
    for (auto entry : serverCustomProperty) {
        netMessage.Append(entry.first);
        netMessage.Append(entry.second);
    }
}
