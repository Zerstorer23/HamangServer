#pragma once

#include "Values.h"
#include "NetworkMessage.h"
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


	static unordered_map<string,string> serverCustomProperty;


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
		newIO->buffer = new char[BUFFER];
		newIO->wsaBuf.len = bufferSize;
		newIO->wsaBuf.buf = newIO->buffer;
		newIO->rwMode = rwMode; // IOCP ��ȣ���� ����� ������ ��� ���� �־����
		return newIO;
	}
	static LPPER_IO_DATA CloneBufferData(char* original, int bufferSize, int rwMode) {
		LPPER_IO_DATA cloneIO = new PER_IO_DATA();
		memset(&(cloneIO->overlapped), 0, sizeof(OVERLAPPED));
		cloneIO->buffer = new char[BUFFER];
		memcpy(cloneIO->buffer, original, bufferSize);//
		cloneIO->wsaBuf.len = bufferSize;
		cloneIO->wsaBuf.buf = cloneIO->buffer;//
		cloneIO->rwMode = rwMode;
		return cloneIO;
	}
	LPPER_IO_DATA CreateMessage(string& message, DWORD bytesSend) {
		LPPER_IO_DATA cloneIO = new PER_IO_DATA();
		memset(&(cloneIO->overlapped), 0, sizeof(OVERLAPPED));
		cloneIO->buffer = new char[BUFFER];
		memcpy(cloneIO->buffer, message.c_str(), bytesSend);
		cloneIO->wsaBuf.len = bytesSend;
		cloneIO->wsaBuf.buf = cloneIO->buffer;
		cloneIO->rwMode = WRITE;
		return cloneIO;
	}
	void HandlePlayerJoin(LPPER_HANDLE_DATA handleInfo, SOCKADDR_IN& clientAddress);
	

	void BindAddress(SOCKADDR_IN& servAddr, const char* ip_addr, const  char* port) {
		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		inet_pton(AF_INET, ip_addr, &(servAddr.sin_addr.s_addr));
		servAddr.sin_port = htons(atoi(port));
	}
	static void SetProperty(string key, string value) {
		serverCustomProperty.insert_or_assign(key, value);
	}
	static void PrintProperties() {
		cout << endl;
		for (auto entry : serverCustomProperty) {
			cout << "Server |\t" << entry.first << "|\t" << entry.second << endl;
		}
	}

	static unsigned WINAPI EchoThreadMain(LPVOID CompletionPortIO);
	static void HandleMessage(NetworkMessage& netMessage);
	static void Handle_PropertyRequest(NetworkMessage& netMessage);
	static void Handle_BroadcastString(NetworkMessage& netMessage);
	static void Append(string& s, string& broadcastString);

	string EncodeServerToNetwork() {
		string message = NET_DELIM;
		message = message.append(to_string(serverCustomProperty.size()));
		for (auto entry : serverCustomProperty) {
			message = message.append(NET_DELIM).append(entry.first).append(NET_DELIM).append(entry.second);
		}
		cout << "Room: " << message << endl;
		return message;
	}
	void EncodeServerToNetwork(NetworkMessage & netMessage) {
		netMessage.Append(to_string(serverCustomProperty.size()));
		for (auto entry : serverCustomProperty) {
			netMessage.Append(entry.first);
			netMessage.Append(entry.second);
		}
	}
	void SetSocketSize(SOCKET& socket) {
		int bufSize = BUFFER;
		socklen_t len = sizeof(bufSize);
		setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, sizeof(bufSize));
		setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)&bufSize, sizeof(bufSize));
		int sendBuf, recvBuf;
		getsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)&sendBuf, &len);
		getsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBuf, &len);
		printf("receive buffer size: %d\n", recvBuf);
		printf("send buffer size: %d\n", sendBuf);
	}
	void SetSocketReusable(SOCKET& socket) {
		int option;
		socklen_t optlen = sizeof(option);
		option = TRUE;
		setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, optlen);

	}
	void OpenServer();
};

