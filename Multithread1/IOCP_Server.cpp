#include "IOCP_Server.h"
#include "PlayerManager.h"
PlayerManager IOCP_Server::playerManager;
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


    //
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
        char ipname[128];
        inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
        printf("Connected client IP : %s \n", ipname);
        Player* player = playerManager.CreatePlayer(handleInfo);
      
        //IO Read이벤트 생성후 구독
        LPPER_IO_DATA ioInfo = CreateBufferData(BUFFER,READ);
        WSARecv(handleInfo->clientSocket, &(ioInfo->wsaBuf), 1, (LPDWORD)&receivedBytes, (LPDWORD)&flags, &(ioInfo->overlapped), NULL); //? 이거 역할이?
        //&(ioInfo->overlapped -> GetQueue...의 4번째  ioInfo로 들어감
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
        //WSARecv가 들어오면 

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
            vector<string> tokens = Split(message, '#');
            for (unsigned int i = 0; i < tokens.size(); i++) {
                cout << tokens[i] << endl;
            }
            if (tokens.size() >= 2) {
                //3. 0 1 <- 1번째 코드 int로 읽고 aoi
                int messageInfo = stoi(tokens[1]);
                if (messageInfo == MSG_REQUEST) {
                    //4. request면 2번째 코드 읽고 switch
                    int messageCode = stoi(tokens[2]);

                }
                else if (messageInfo == MSG_HASH) {
                    //SetHash면 서버 Hash도 업데이트 필요
                    //actorNum, SetHash [int]roomOrPlayer [string]Key [object]value
                }
                else {
                    //그외에는 BroadCast
                    playerManager.BroadcastMessage(sourcePlayer->actorNumber, receivedIO, bytesReceived);
                }
            
            }
            else {
                playerManager.BroadcastMessage(sourcePlayer->actorNumber, receivedIO, bytesReceived);
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
