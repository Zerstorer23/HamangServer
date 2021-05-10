#pragma once

#include "Values.h"
class PlayerManager;
class ServerExample {
public:
		string ipAddress;
		string portNumber;
		SOCKET serverSocket;
		SOCKADDR_IN servAddr;
		int clientCount = 0;
	//	SOCKET clientSockets[MAX_CLIENT];
		static PlayerManager playerManager;
		
	


public:
	ServerExample() {
		serverSocket = NULL;
	}
	~ServerExample() {
	
	}

	

	void CloseServer() {
	

		closesocket(serverSocket);
		WSACleanup();
	}
	void OpenServer();

	int InitialiseServer(char* ip, char* port) {
		WSADATA wsaData;
		ipAddress = ip;
		portNumber = port;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			printf("Start up error");
			return -1;
		}
		return 0;
	}
	static  unsigned WINAPI HandleClient(void* arg);
	void SendMessage(char* message, int len);

	void BindAddress(SOCKADDR_IN& servAddr, const char* ip_addr, const char* port) {
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		inet_pton(AF_INET, ip_addr, &(servAddr.sin_addr.s_addr));
		servAddr.sin_port = htons(atoi(port));
	}
};