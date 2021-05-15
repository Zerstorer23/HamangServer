#include "IOCP_Server.h"
#include "PlayerManager.h"
#include "NetworkMessage.h"
PlayerManager IOCP_Server::playerManager;
unordered_map<string, string> IOCP_Server::serverCustomProperty;
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
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL); //? 이거 역할이?
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

    //1. Send All player information with name and hash
            /*
         0 Sent Actor Num = -1
         1 MessageInfo = Callback
         3. sub info = join local
         4 RoomInfo Begin==
           4 - NumRoomHash
                1 - key (int)
                2 - value (string)
         5. Player Begin===
            5. NumPlayer
            6. LocalPlayer
               6. id, ismaster, numParam , key..value...

         */
         //params = [int]numPlayers(local Included) , LocalPlayerInfo , players[...
         //Player Info = actorID, isMaster, customprop[num prop]
    string message =to_string(0).append(to_string(-1)).append(NET_DELIM).append(to_string((int)MessageInfo::ServerCallbacks)).append(NET_DELIM).append(to_string((int)LexCallback::LocalPlayerJoined));
    cout << message << endl;
    //RoomInfo start
    message = message.append(EncodeServerToNetwork());
    cout << message << endl;
    //Players
    message = message.append(playerManager.EncodePlayersToNetwork(player));
    DWORD bytesSend = message.length() +1;
    cout << "Sent bytes " << bytesSend << endl;
    LPPER_IO_DATA sendIO = CreateMessage(message, bytesSend);
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
                SAFE_DELETE(receivedIO)
                playerManager.RemovePlayer(sourcePlayer->actorNumber);
                //Callback": Disconnect 송신
                continue;
            }
            //1. Contents
            cout << "Received Message :" << receivedIO->buffer << endl;
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
                char sendBuffer[BUFFER];
                string msg = netMessage.Build();
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
        bool isMyPacket =(   signature.compare(SIGNATURE) == 0);
        if (!isMyPacket) continue;

        //2. 메세지 길이 읽기
        int lengthOfMessages = stoi(netMessage.GetNext());
            //3. 메세지 발언자 읽기
        int senderID = stoi(netMessage.GetNext());
        //4. 메세지 타입 읽기
        MessageInfo messageInfo = (MessageInfo)stoi(netMessage.GetNext());
        int endPointOfAMessage = beginPointOfAMessage + lengthOfMessages;
        cout << signature << "Is my packet from "<<beginPointOfAMessage<<" to "<<endPointOfAMessage << endl;
        if (messageInfo == MessageInfo::ServerRequest) {
            //4. request면 2번째 코드 읽고 switch
            int messageCode = stoi(netMessage.GetNext());
        }
        else if (messageInfo == MessageInfo::SetHash) {
            cout << "Handle hash : " << (int)messageInfo << endl;
            Handle_PropertyRequest(netMessage);
            netMessage.SaveStrings(beginPointOfAMessage, endPointOfAMessage);
        }
        else {
            //0 /1 2 3 4/ 5 6
            netMessage.SaveStrings(beginPointOfAMessage, endPointOfAMessage);//end exclusive
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
        IOCP_Server::SetProperty(key, value);
        IOCP_Server::PrintProperties();
    }
    else {
        playerManager.playerHash[target]->SetProperty(key, value);
        playerManager.playerHash[target]->PrintProperties();
    }
}

