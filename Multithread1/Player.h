#pragma once
#include "Values.h"

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

	//void Send(char* message, int len) {
	//	printf("sent: %s %d\n", message, strlen(message));
	//	printf("expected send bytes: %d\n", len);

	//	//send(clientSocket, (char*)&len, sizeof(len), 0);
	////	send(clientSocket, message, len, 0);
	//}


};

