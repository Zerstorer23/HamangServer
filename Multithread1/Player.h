#pragma once
#include "Values.h"
class NetworkMessage;
class IOCP_Server;
class HashTable;
class Player
{

public:
	LPPER_HANDLE_DATA handleInfo;
	string unique_id;
	int actorNumber;
	bool isMasterClient;
	shared_ptr<HashTable> customProperty;

	Player();
	~Player() {
		//handleInfo는 Server.cpp가 삭제함

	}
	void SetActorNumber(int id);
	//void SetProperty(string key, string typeName, string value) {
	//	customProperty.Set
	//}
	//void PrintProperties() {
	//	cout << endl;
	//	for (auto entry : CustomProperty) {
	//		cout << "Client"<<actorNumber <<" |\t" << entry.first << "|\t" << entry.second << endl;
	//	}
	//}
	void Send(char* sendBuffer, DWORD& bytesReceived);
	void Send(LPPER_IO_DATA sendIO);

	void EncodeToNetwork(NetworkMessage& netMessage);
};

