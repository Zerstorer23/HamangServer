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
#define BUFFER 1024*32
#define READ 3
#define WRITE 5
#define SIGNATURE "LEX"

#define MAX_CLIENT 20
#define	SAFE_DELETE(p)	if(p)	{ delete p; p = nullptr; }
using namespace std;

class Player;
typedef struct {
	SOCKET clientSocket;
	SOCKADDR_IN clientAddress;
	Player* player;
}PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct {
	OVERLAPPED overlapped;//구조체의 주소는 첫변수 overlap의주소
	WSABUF wsaBuf;//WSAREcv
	char buffer[BUFFER]; //=malloc() new char[](BUFFER)
	int rwMode;
}PER_IO_DATA, * LPPER_IO_DATA;

/*
1. 모든 cpp는 메인 헤더
2. 자기 cpp는 자기 헤더
3. 자기 cpp에서 다른 cpp는 헤더에 전방선언 후 cpp에서 헤더 include.

*/