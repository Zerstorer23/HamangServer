#pragma once
#include "Values.h"
#include "IOCP_Server.h"
#include "NetworkMessage.h"
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
	void Send(char* sendBuffer, DWORD & bytesReceived) {
		LPPER_IO_DATA cloneIO = IOCP_Server::CloneBufferData(sendBuffer, bytesReceived, WRITE);
		SOCKET targetSocket = handleInfo->clientSocket;
		cout <<sendBuffer<< "Send to Actor number " << actorNumber << endl;
		WSASend(targetSocket, &(cloneIO->wsaBuf), 1, NULL, 0, &(cloneIO->overlapped), NULL);
	}
	void Send(LPPER_IO_DATA sendIO) {
		SOCKET targetSocket = handleInfo->clientSocket;
		cout << "Send to Actor number " << actorNumber << endl;
		WSASend(targetSocket, &(sendIO->wsaBuf), 1, NULL, 0, &(sendIO->overlapped), NULL);
	}
	/*string EncodeToNetwork() {
		string message = NET_DELIM;
		message = message.append(to_string(actorNumber)).append(NET_DELIM)
			.append(to_string(isMasterClient)).append(NET_DELIM)
			.append(to_string(CustomProperty.size()));

		for (auto entry : CustomProperty) {
			message = message.append(NET_DELIM).append(entry.first).append(NET_DELIM).append(entry.second);
		}
		return message;
	}*/
	void EncodeToNetwork(NetworkMessage & netMessage) {
		netMessage.Append(to_string(actorNumber));
		netMessage.Append(to_string(isMasterClient));
		netMessage.Append(to_string(CustomProperty.size()));
		for (auto entry : CustomProperty) {
			netMessage.Append(entry.first);
			netMessage.Append(entry.second);
		}
	}
};

