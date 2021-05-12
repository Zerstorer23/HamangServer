#pragma once
#include "Values.h"
#include "IOCP_Server.h"
class Player
{

public:
	LPPER_HANDLE_DATA handleInfo;
	string unique_id;
	int actorNumber;
	bool isHost;
	Player() {
		handleInfo = nullptr;
		actorNumber = -1;
		unique_id = "";
		isHost = false;

	}
	~Player() {
		//handleInfo는 Server.cpp가 삭제함

	}

	void Send(LPPER_IO_DATA receivedIO, DWORD & bytesReceived) {
		LPPER_IO_DATA cloneIO = IOCP_Server::CloneBufferData(receivedIO, bytesReceived, WRITE);
		SOCKET targetSocket = handleInfo->clientSocket;
		cout << "Send to Actor number " << actorNumber << endl;
		WSASend(targetSocket, &(cloneIO->wsaBuf), 1, NULL, 0, &(cloneIO->overlapped), NULL);
	}


};

