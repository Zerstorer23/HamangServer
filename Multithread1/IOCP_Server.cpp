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
        CreateIoCompletionPort((HANDLE)clientSocket, hCompletionPort, (ULONG_PTR)handleInfo, 0); //원래 DWORD였음
        //client에 대한 io가 완료되면 정보가 hCompletionPort에 등록됨
        //handleInfo는 정보 매개변수 ->getQueuedCopletionStatus의 세번쨰 인자로 전달됨

        ///->이 단계에서 소켓 리스트를 만들 필요가 있음
        char ipname[128];
        inet_ntop(AF_INET, (void*)&clientAddress.sin_addr, (PSTR)ipname, sizeof(ipname));
        printf("Connected client IP : %s \n", ipname);
        Player* player = playerManager.CreatePlayer(handleInfo);
        //
        ioInfo = new PER_IO_DATA();
        memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
        ioInfo->wsaBuf.len = BUFFER;
        ioInfo->wsaBuf.buf = ioInfo->buffer;
        ioInfo->rwMode = READ; // IOCP 신호에는 입출력 구분이 없어서 직접 넣어야함
        //READ만 담당

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
    LPPER_IO_DATA ioInfo;
    while (true) {
        GetQueuedCompletionStatus(hCompletionPort, &bytesReceived, (PULONG_PTR)&handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
        //WSARecv가 들어오면 

        Player * sourcePlayer = handleInfo->player;

        clientSocket = handleInfo->clientSocket;
        if (ioInfo->rwMode == READ) {
            if (bytesReceived == 0) {//종료
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
            //송신모드
            //--> 이 단계에서 모든 소켓에 보낼 필요가 있음
            for (auto entry : playerManager.playerHash) {
                Player* targetPlayer = entry.second;
                if (targetPlayer->actorNumber == sourcePlayer->actorNumber) continue;
                SOCKET targetSocket = targetPlayer->handleInfo->clientSocket;
                // LPPER_IO_DATA target_ioInfo = entry.second->;
                cout << "Actor number " << entry.second->actorNumber << endl;
                //2.Contents //Broadcast안됨
               // send(targetSocket, (ioInfo->wsaBuf.buf), (ioInfo->wsaBuf.len), 0);
                WSASend(targetSocket, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
            }


            // 다시 읽기모드

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
                 delete ioInfo; //TODO 이거 지우면 안되는데
                 ioInfo = nullptr; //ioInfo 생성 삭제에 대해서 알아봐야함
             }*/
        }
    }
    return 0;

}
