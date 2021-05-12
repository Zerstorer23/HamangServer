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
	SOCKET serverSocket;
	SOCKADDR_IN serverAddress;


public:
	IOCP_Server() {
		wsaData = {};
		sysInfo = {};
		serverSocket = {};
		hCompletionPort = {};
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
	LPPER_IO_DATA CreateBufferData(int bufferSize, int rwMode) {
		LPPER_IO_DATA newIO = new PER_IO_DATA();
		memset(&(newIO->overlapped), 0, sizeof(OVERLAPPED));
		newIO->wsaBuf.len = bufferSize;
		newIO->wsaBuf.buf = newIO->buffer;
		newIO->rwMode = rwMode; // IOCP 신호에는 입출력 구분이 없어서 직접 넣어야함
		return newIO;
	}
	static LPPER_IO_DATA CloneBufferData(LPPER_IO_DATA original, int bufferSize, int rwMode) {
		LPPER_IO_DATA cloneIO = new PER_IO_DATA();
		memset(&(cloneIO->overlapped), 0, sizeof(OVERLAPPED));
		memcpy(cloneIO->buffer, original->buffer, bufferSize);
		cloneIO->wsaBuf.len = bufferSize;
		cloneIO->wsaBuf.buf = original->buffer;
		cloneIO->rwMode = rwMode;
		return cloneIO;
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

