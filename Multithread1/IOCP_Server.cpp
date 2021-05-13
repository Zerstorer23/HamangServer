#include "IOCP_Server.h"
#include "PlayerManager.h"
PlayerManager IOCP_Server::playerManager;
unordered_map<string, string> IOCP_Server::serverCustomProperty;
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
        HandlePlayerJoin(handleInfo,clientAddress);

      
        //IO Read�̺�Ʈ ������ ����
        LPPER_IO_DATA ioInfo = CreateBufferData(BUFFER,READ);
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL); //? �̰� ������?
        //&(ioInfo->overlapped -> GetQueue...�� 4��°  ioInfo�� ��
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
            bool broadcastMessage =  HandleMessage(tokens);
            if (broadcastMessage) {
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

void IOCP_Server::Handle_PropertyRequest(vector<string>& tokens)
{
    //SetHash�� ���� Hash�� ������Ʈ �ʿ�
    //actorNum, SetHash [int]roomOrPlayer [string]Key [object]value
    int target = stoi(tokens[2]);
    string key = tokens[3];
    string value = tokens[4];
    if (target == 0) {
        IOCP_Server::SetProperty(key,value);
        IOCP_Server::PrintProperties();
    }
    else {
        playerManager.playerHash[target]->SetProperty(key, value);
        playerManager.playerHash[target]->PrintProperties();
    }
}

bool IOCP_Server::HandleMessage(vector<string>& tokens)
{
    if (tokens.size() >= 2) {
        //3. 0 1 <- 1��° �ڵ� int�� �а� aoi
        MessageInfo messageInfo = (MessageInfo)stoi(tokens[1]);
        if (messageInfo == MessageInfo::ServerRequest) {
            //4. request�� 2��° �ڵ� �а� switch
            int messageCode = stoi(tokens[2]);
            return false;
        }
        else if (messageInfo == MessageInfo::SetHash) {
            cout << "Handle hash : " << (int)messageInfo << endl;
            Handle_PropertyRequest(tokens);
            return true;
        }
        else {
            //�׿ܿ��� BroadCast
            return true;
        }
    }
    else {
        return true;
    }


}
