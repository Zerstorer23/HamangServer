#pragma once

#include "Values.h"

class PlayerManager;
class IOCP_Server
{
	string ipAddress;
	string portNumber;
	int clientCount = 0;
	//	SOCKET clientSockets[MAX_CLIENT];
	static PlayerManager playerManager;

	WSADATA wsaData;
	HANDLE hCompletionPort;
	SYSTEM_INFO sysInfo;
	LPPER_IO_DATA ioInfo;
	LPPER_HANDLE_DATA handleInfo;
	SOCKET serverSocket;
	SOCKADDR_IN serverAddress;


public:
	IOCP_Server() {
		wsaData = {};
		sysInfo = {};
		serverSocket = {};
		hCompletionPort = {};
		handleInfo = {};
		ioInfo = {};
		serverAddress = {};
	}
	~IOCP_Server() {
	}
	void InitialiseServer(string ip, string port) {
		WSADATA wsaData;
		ipAddress = ip;
		portNumber = port;

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			printf("Start up error");
			exit(1);
		}
	}

	void CloseServer() {
		closesocket(serverSocket);
		WSACleanup();
	}
	void BindAddress(SOCKADDR_IN& servAddr, const char* ip_addr, const  char* port) {
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		inet_pton(AF_INET, ip_addr, &(servAddr.sin_addr.s_addr));
		servAddr.sin_port = htons(atoi(port));
	}
	static unsigned WINAPI EchoThreadMain(LPVOID CompletionPortIO);
	void OpenServer();
};

