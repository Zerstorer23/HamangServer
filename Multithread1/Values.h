#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <process.h>
#include <cassert>
#include <unordered_map>
#include <string>
#include "SharedEnums.h"
#include <queue>
#include <chrono>
#include <ctime>
#include <algorithm> // remove and remove_if
#include <thread>
#include <wchar.h>
#include <locale>
#include <locale.h>

using namespace std;
using chrono::duration_cast;
using chrono::milliseconds;
using chrono::seconds;
using chrono::system_clock;
#define BUFFER 1024*32
#define READ 3
#define WRITE 5

#define MAX_CLIENT 20
#define	SAFE_DELETE(p)	if(p)	{ delete p; p = nullptr; }
#define SAFE_DELETE_ARRAY(p) if(p) {delete[] p; p=NULL;}

#define DECLARE_SINGLE(Type)	\
	private:\
		static Type* instance; \
	public:\
		static Type* GetInst(){\
			if(!instance)\
			instance = new Type;\
			return instance;\
			}\
		static void DestroyInst() {	\
			SAFE_DELETE(instance);	\
		}							\
	private:					\
		Type();					\
		~Type();				

#define DEFINITION_SINGLE(Type) Type* Type::instance = NULL;
#define GET_SINGLE(Type) Type::GetInst()
#define DESTROY_SINGLE(Type) Type::DestroyInst()

class Player;
typedef struct {
	SOCKET clientSocket;
	SOCKADDR_IN clientAddress;
	Player* player;
}PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct {
	OVERLAPPED overlapped;//����ü�� �ּҴ� ù���� overlap���ּ�
	WSABUF wsaBuf;//WSAREcv
	//char buffer[BUFFER]; //=malloc() new char[](BUFFER)
	char* buffer; //= nullptr;// = new char[BUFFER];
	int rwMode;
}PER_IO_DATA, * LPPER_IO_DATA;
typedef struct {
	int playerActorNr;
	int viewID;
	wstring message;
}RPC, * PRPC;



/*
1. ��� cpp�� ���� ���
2. �ڱ� cpp�� �ڱ� ���
3. �ڱ� cpp���� �ٸ� cpp�� ����� ���漱�� �� cpp���� ��� include.

*/