#pragma once
#include "Values.h"
class Player
{

public:

	SOCKET clientSocket;
	string unique_id;
	int actorNumber;
	bool isHost;
	Player() {
		clientSocket = NULL;
		actorNumber = -1;
		unique_id ="";
		isHost = false;
	
	}
	~Player() {
	
	}

	void Send(char* message, int len) {
		printf("sent: %s %d\n", message, strlen(message));
		printf("expected send bytes: %d\n", len);

		send(clientSocket, (char*)&len, sizeof(len), 0);
		send(clientSocket, message, len, 0);
	}


};

