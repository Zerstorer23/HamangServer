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
		newIO->wsaBuf.len = bufferSize;
		newIO->wsaBuf.buf = newIO->buffer;
		newIO->rwMode = rwMode; // IOCP ��ȣ���� ����� ������ ��� ���� �־����
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
	static bool HandleMessage(vector<string>& tokens);
	static void Handle_PropertyRequest(vector<string>& tokens);

	string EncodeServerToNetwork() {
		string message = NET_DELIM;
		message = message.append(to_string(serverCustomProperty.size()));
		for (auto entry : serverCustomProperty) {
			message = message.append(NET_DELIM).append(entry.first).append(NET_DELIM).append(entry.second);
		}
		cout << "Room: " << message << endl;
		return message;
	}
	LPPER_IO_DATA CreateMessage(string& message, DWORD bytesSend) {
		LPPER_IO_DATA cloneIO = new PER_IO_DATA();
		memset(&(cloneIO->overlapped), 0, sizeof(OVERLAPPED));
		memcpy(cloneIO->buffer, message.c_str(), bytesSend);
		cloneIO->wsaBuf.len = bytesSend;
		cloneIO->wsaBuf.buf = cloneIO->buffer;
		cloneIO->rwMode = WRITE;
		return cloneIO;
	}

	void OpenServer();
};

