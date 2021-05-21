#pragma once
#include "Values.h"
class NetworkMessage;
class IOCP_Server;
class Player
{

public:
	LPPER_HANDLE_DATA handleInfo;
	string unique_id;
	int actorNumber;
	bool isMasterClient;
	unordered_map<string, string> CustomProperty;

	Player() {
		handleInfo = nullptr;
		actorNumber = -1;
		unique_id = "";
		isMasterClient = false;

	}
	~Player() {
		//handleInfo는 Server.cpp가 삭제함

	}
	void SetProperty(string key, string value) {
		CustomProperty.insert_or_assign(key, value);
	}
	void PrintProperties() {
		cout << endl;
		for (auto entry : CustomProperty) {
			cout << "Client"<<actorNumber <<" |\t" << entry.first << "|\t" << entry.second << endl;
		}
	}
	void Send(char* sendBuffer, DWORD& bytesReceived);
	void Send(LPPER_IO_DATA sendIO) {
		SOCKET targetSocket = handleInfo->clientSocket;
		cout << "Send to Actor number " << actorNumber << endl;
		WSASend(targetSocket, &(sendIO->wsaBuf), 1, NULL, 0, &(sendIO->overlapped), NULL);
	}

	void EncodeToNetwork(NetworkMessage& netMessage);
};

