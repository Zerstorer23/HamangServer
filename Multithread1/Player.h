#pragma once
#include "Values.h"
#include "IOCP_Server.h"
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
	void Send(LPPER_IO_DATA receivedIO, DWORD & bytesReceived) {
		LPPER_IO_DATA cloneIO = IOCP_Server::CloneBufferData(receivedIO, bytesReceived, WRITE);
		SOCKET targetSocket = handleInfo->clientSocket;
		cout << "Send to Actor number " << actorNumber << endl;
		WSASend(targetSocket, &(cloneIO->wsaBuf), 1, NULL, 0, &(cloneIO->overlapped), NULL);
	}
	void Send(LPPER_IO_DATA sendIO) {
		SOCKET targetSocket = handleInfo->clientSocket;
		cout << "Send to Actor number " << actorNumber << endl;
		WSASend(targetSocket, &(sendIO->wsaBuf), 1, NULL, 0, &(sendIO->overlapped), NULL);
	}
	string EncodeToNetwork() {
	/*
	        this.isLocal = isLocal;
        CustomProperties = new Dictionary<PlayerProperty, string>();
        this.actorID = Int32.Parse(stringQueue.Dequeue());
        this.IsMasterClient = Boolean.Parse(stringQueue.Dequeue());
        int numParam = Int32.Parse(stringQueue.Dequeue());
        int i = 0;
        Debug.Log(string.Format("Received Player {1}, isMaster{2}", actorID, IsMasterClient));
        while (i < numParam) {
            PlayerProperty key =(PlayerProperty) Int32.Parse(stringQueue.Dequeue());
            string value = stringQueue.Dequeue();
            Debug.Log("Key " + key + " / " + value);
            CustomProperties.Add(key, value);
            i++;
        }
        NickName = CustomProperties[PlayerProperty.NickName];
	*/
		string message = NET_DELIM;
		message = message.append(to_string(actorNumber)).append(NET_DELIM)
			.append(to_string(isMasterClient)).append(NET_DELIM)
			.append(to_string(CustomProperty.size()));

		for (auto entry : CustomProperty) {
			message = message.append(NET_DELIM).append(entry.first).append(NET_DELIM).append(entry.second);
		}
		return message;
	}
};

