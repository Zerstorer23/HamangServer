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
using namespace string_literals;
using chrono::system_clock;
/*
TODO 별도로 Merge 과저없이도 buffer에 뭉쳐서 들어오고 나가는 경우가 존재.
이 경우 buffer에서 잘리는 데이터가 존재함.

1024 1024하니까 bad_alloc나옴
*/
#define BUFFER 128 * 1024 //128에서 성공
#define READ 3
#define WRITE 5
#define EASY_LOG 0
#define CRITICAL_LOG 0

#define MAX_CLIENT 20
#define	SAFE_DELETE(p)	if(p)	{ delete p; p = nullptr; }
#define SAFE_DELETE_ARRAY(p) if(p) {delete[] p; p=NULL;}
#define DEBUG_MODE if(CRITICAL_LOG)


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
	OVERLAPPED overlapped;//구조체의 주소는 첫변수 overlap의주소
	WSABUF wsaBuf;//WSAREcv
	//char buffer[BUFFER]; //=malloc() new char[](BUFFER)
	char* buffer; //= nullptr;// = new char[BUFFER];
	int rwMode;
}PER_IO_DATA, * LPPER_IO_DATA;
typedef struct {
	int playerActorNr;
	int viewID;
	string message;
}RPC, * PRPC;



/*
1. 모든 cpp는 메인 헤더
2. 자기 cpp는 자기 헤더
3. 자기 cpp에서 다른 cpp는 헤더에 전방선언 후 cpp에서 헤더 include.

*/