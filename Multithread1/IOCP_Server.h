#pragma once

#include "Values.h"
class NetworkMessage;
class PlayerManager;
class PingManager;
class BufferedMessages;
class MessageHandler;
class HashTable;
class IOCP_Server
{
private:
	static IOCP_Server * serverInstance;
public:
	string ipAddress;
	string portNumber;
	int clientCount = 0;
	//	SOCKET clientSockets[MAX_CLIENT];
	static BufferedMessages bufferedRPCs;
	WSADATA wsaData;
	HANDLE hCompletionPort;
	SYSTEM_INFO sysInfo;
	SOCKET serverSocket;
	SOCKADDR_IN serverAddress;

	shared_ptr<HashTable> customProperty;

	static MessageHandler messageHandler;

public:
	static IOCP_Server * GetInst() {
		if (!serverInstance) {
			serverInstance = new IOCP_Server;
			return serverInstance;
		}
		else {
			return serverInstance;
		}
	}
	static void DestroyInst() {
		SAFE_DELETE(serverInstance);
	}
private:
	IOCP_Server();
	~IOCP_Server() {
	};
public:
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
	static LPPER_IO_DATA CreateEmptyBuffer(int bufferSize, int rwMode) {
		LPPER_IO_DATA newIO = new PER_IO_DATA();
		memset(&(newIO->overlapped), 0, sizeof(OVERLAPPED));
		newIO->buffer = new char[BUFFER];
		newIO->wsaBuf.len = bufferSize;
		newIO->wsaBuf.buf = newIO->buffer;
		newIO->rwMode = rwMode; // IOCP 신호에는 입출력 구분이 없어서 직접 넣어야함
		return newIO;
	}
	LPPER_IO_DATA CreateMessageBuffer(string u8message, int rwMode) {
		char* original =(char * ) u8message.c_str();
		int bufferSize = u8message.size();
		/*
		* TODO std::bad_alloc
		https://ju3un.github.io/c++-new-excepiton/
		*/
		LPPER_IO_DATA cloneIO = new PER_IO_DATA();
		memset(&(cloneIO->overlapped), 0, sizeof(OVERLAPPED));
		cloneIO->buffer = new char[BUFFER];
		memcpy(cloneIO->buffer, original, bufferSize);//
		cloneIO->wsaBuf.len = bufferSize;
		cloneIO->wsaBuf.buf = cloneIO->buffer;//
		cloneIO->rwMode = rwMode;
		return cloneIO;
	}
	static void RecycleIO(LPPER_IO_DATA receivedIO, int rwMode) {
		memset(&(receivedIO->overlapped), 0, sizeof(OVERLAPPED));
		receivedIO->wsaBuf.len = BUFFER;
		receivedIO->wsaBuf.buf = receivedIO->buffer;
		receivedIO->rwMode = rwMode;
	}

	void HandlePlayerJoin(LPPER_HANDLE_DATA handleInfo, SOCKADDR_IN& clientAddress);
	static void HandlePlayerDisconnect(LPPER_IO_DATA receivedIO, LPPER_HANDLE_DATA handleInfo, Player* sourcePlayer);



	void BindAddress(SOCKADDR_IN& servAddr, const char* ip_addr, const  char* port) {
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		//	inet_pton(AF_INET, ip_addr, &(servAddr.sin_addr.s_addr));
		servAddr.sin_port = htons(atoi(port));
	}


	static unsigned WINAPI EchoThreadMain(LPVOID CompletionPortIO);
	static void Append(wstring& s, wstring& broadcastString);
	
	void SetSocketSize(SOCKET& socket) {
		int bufSize = BUFFER;
		int sendBuffer = 0; // recv 0 아니면 buffer
		//send는 0이어도 됨..
		//recv
		//다 안차면 수신대기상태... <- 0
		//recv없이 얼마나 send되는지
		socklen_t len = sizeof(bufSize);
		//setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, sizeof(bufSize));
		setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)&bufSize, sizeof(bufSize));
		//int sendBuf,
		int recvBuf;
		//getsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBuf, &len);
		getsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBuf, &len);
		printf("receive buffer size: %d\n", recvBuf);
		//printf("send buffer size: %d\n", sendBuf);
	}
	void SetSocketReusable(SOCKET& socket) {
		int option;
		socklen_t optlen = sizeof(option);
		option = TRUE;
		setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, optlen);

	}
	void OpenServer();

	void ResetServer();



};

