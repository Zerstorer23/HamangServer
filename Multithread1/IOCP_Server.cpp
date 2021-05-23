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
    //1. CP오브젝트 생성. 마지막인자 0 -> 코어의 수만큼 cp오브젝트에 스레드를 할당
    hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);//마지막인자만 중요
    GetSystemInfo(&sysInfo);
    for (unsigned int i = 0; i < sysInfo.dwNumberOfProcessors; i++) {
        //코어 수만큼 스레드 생성
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
        CreateIoCompletionPort((HANDLE)clientSocket, hCompletionPort, (ULONG_PTR)handleInfo, 0); //원래 DWORD였음 struct늘리니까 LONG달라함
        //client에 대한 io가 완료되면 정보가 hCompletionPort에 등록됨
        //handleInfo는 정보 매개변수 ->getQueuedCopletionStatus의 세번쨰 인자로 전달됨

        ///->이 단계에서 소켓 리스트를 만들 필요가 있음
        HandlePlayerJoin(handleInfo,clientAddress);

      
        //IO Read이벤트 생성후 구독
        LPPER_IO_DATA ioInfo = CreateBufferData(BUFFER,READ);
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL);
        //&(ioInfo->overlapped -> GetQueue...의 4번째  ioInfo로 들어감
    }
    return;
}
void IOCP_Server::HandlePlayerJoin(LPPER_HANDLE_DATA handleInfo, SOCKADDR_IN& clientAddress) {
    char ipname[128];
    inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
    printf("Connected client IP : %s \n", ipname);
    Player* player = playerManager.CreatePlayer(handleInfo);
    playerManager.PrintPlayers();
    NetworkMessage netMessage;
    netMessage.Append("-1");
    netMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    netMessage.Append(to_string((int)LexCallback::RoomInformationReceived));
    EncodeServerToNetwork(netMessage);
    //Players
    playerManager.EncodePlayersToNetwork(player, netMessage);
    //message = message.append(playerManager.EncodePlayersToNetwork(player));
    string message = netMessage.BuildNewSignedMessage();
    LPPER_IO_DATA sendIO = CreateMessage(message);
    cout << "IO Created" << endl;
    player->Send(sendIO);
}

/*
 1024 < - 600 / 400 200
 
 SETSOCKET... 1mb

 //a 1 2 3
 //  1[2..1..3] 2 3
 
 
 랜카드 버퍼  == 
 
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
        //WSARecv가 들어오면 
        //40 30 // 40 10더
        //Shared pointer
        Player * sourcePlayer = handleInfo->player;

        clientSocket = handleInfo->clientSocket;
        if (receivedIO->rwMode == READ) {
            if (bytesReceived == 0) {//종료
                cout << "Disconnect Client" << endl;
                SAFE_DELETE(handleInfo)
                SAFE_DELETE(receivedIO->buffer)
                SAFE_DELETE(receivedIO)
                playerManager.RemovePlayer(sourcePlayer->actorNumber);
                //Callback": Disconnect 송신
                continue;
            }
            //1. Contents
           // cout << "Received Message :" << receivedIO->buffer << endl;
            //ServerRequest면 이 단계에서 송신
            //C#에서 받으니까 첫번째에 이상한 캐릭터 있음
                    //1. 첫자 지우고
            string message = receivedIO->buffer;
            message.erase(0,1);

            cout << "Cleansed Message :" << message << endl;
            //2. split하고
            NetworkMessage netMessage;
            netMessage.Split(message, '#');
            HandleMessage(netMessage);
            if (netMessage.HasMessageToBroadcast()) {
                string msg = netMessage.BuildCopiedMessage();
               // playerManager.BroadcastMessage(sourcePlayer->actorNumber, sendBuffer, bytesReceived);
                playerManager.BroadcastMessageAll((char *)msg.c_str(), bytesReceived);
            }
           
            

     


            // 다시 읽기모드로 재활용
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



void IOCP_Server::HandleMessage(NetworkMessage & netMessage)
{
    while (netMessage.HasNext()) {
        int beginPointOfAMessage = netMessage.iterator;
        //1. Check signature
        string signature = netMessage.GetNext();
        bool isMyPacket =(   signature.compare(NET_SIG) == 0);
        if (!isMyPacket) continue;

        //2. 메세지 길이 읽기
        int lengthOfMessages = stoi(netMessage.GetNext());
        //3. 메세지 발언자 읽기
        netMessage.sentActorNr = stoi(netMessage.GetNext());
        //4. 메세지 타입 읽기
        MessageInfo messageInfo = (MessageInfo)stoi(netMessage.GetNext());
        int endPointOfAMessage = beginPointOfAMessage + lengthOfMessages;
        cout << signature << "Is my packet from "<<beginPointOfAMessage<<" to "<<endPointOfAMessage << endl;
        if (messageInfo == MessageInfo::ServerRequest) {
            cout << "Received server request" << endl;
            //4. request면 2번째 코드 읽고 switch
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
            //한단위씩 저장됨
            string message =  netMessage.SaveStrings(beginPointOfAMessage, endPointOfAMessage);//end exclusive
            cout << "Received server request" <<message<< endl;
            if (messageInfo == MessageInfo::RPC || messageInfo == MessageInfo::Instantiate 
                || messageInfo == MessageInfo::Destroy || messageInfo == MessageInfo::SyncVar) {
                //모든 방송메세지는 0 sig / 1 len / 2 sender / 3 type / 4 viewID 형식
                //저장이 필요한 타입들
                netMessage.targetViewID = stoi(netMessage.GetNext());
                bufferedRPCs.EnqueueMessage(netMessage.sentActorNr, netMessage.targetViewID, message);
            }
            netMessage.iterator = endPointOfAMessage;
        }
    }
   


}

void IOCP_Server::Handle_PropertyRequest(NetworkMessage& netMessage)
{
    //SetHash면 서버 Hash도 업데이트 필요
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
    case LexRequest::RemoveRPC_ViewID:
        break;
    case LexRequest::RemoveRPC_Player:
        break;
    case LexRequest::Receive_RPCbuffer:
        Handle_ServerRequest_SendBufferedRPCs(netMessage);
        break;
    case LexRequest::Receive_modifiedTime:
        Handle_ServerRequest_ModifyTime(netMessage);
        break;
    }

}
void IOCP_Server::Handle_ServerRequest_SendBufferedRPCs(NetworkMessage& netMessage) {
    Player * target = playerManager.playerHash[netMessage.sentActorNr];
    bufferedRPCs.SendBufferedMessages(target);

    //TODO 이게 먼저도착할거같은데
    //EOL 보내기
    NetworkMessage eolMessage;
    eolMessage.Append(to_string(netMessage.sentActorNr));
    eolMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
    eolMessage.Append(to_string((int)LexCallback::BufferedRPCsLoaded));
    string message = eolMessage.BuildNewSignedMessage();
    LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
    target->Send(sendIO);
    cout << "Sent buffered RPCs" << endl;
}

void IOCP_Server::Handle_ServerRequest_ModifyTime(NetworkMessage& netMessage) {
    pingManager.Handle_Request_TimeSynch(netMessage);
};
void IOCP_Server::EncodeServerToNetwork(NetworkMessage& netMessage) {
    netMessage.Append(to_string(serverCustomProperty.size()));
    for (auto entry : serverCustomProperty) {
        netMessage.Append(entry.first);
        netMessage.Append(entry.second);
    }
}
