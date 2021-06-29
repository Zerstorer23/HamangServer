#pragma once
#include "Values.h"
class NetworkMessage;
class IOCP_Server;
class HashTable;
class Player
{

public:
	LPPER_HANDLE_DATA handleInfo;
	wstring unique_id;
	int actorNumber;
	bool isMasterClient;
	bool isConnected;
	shared_ptr<HashTable> customProperty;

	Player();
	~Player() {
		//handleInfo는 Server.cpp가 삭제함

	}
	void SetActorNumber(int id);

//	void Send(char* sendBuffer, DWORD& bytesReceived);
	//void Send(LPPER_IO_DATA sendIO);
	void Send(wstring message, bool isInitial);

	void EncodeToNetwork(NetworkMessage& netMessage);
};

