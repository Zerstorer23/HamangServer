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

#define BUFFER 1024
#define READ 3
#define WRITE 5


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
	WSABUF wsaBuf;
	char buffer[BUFFER];
	int rwMode;

}PER_IO_DATA, * LPPER_IO_DATA;
static vector<string> Split(string& str, char delim) {
	int previous = 0;
	int current = 0;
	vector<string> x;
	x.clear();

	current = str.find(delim);
	//find는 찾을게 없으면 npos를 리턴함
	while (current != string::npos) {
		string substring = str.substr(previous, current - previous);
		x.push_back(substring);
		previous = current + 1;
		current = str.find(delim, previous);
	}
	x.push_back(str.substr(previous, current - previous));
	return x;
}
/*
1. 모든 cpp는 메인 헤더
2. 자기 cpp는 자기 헤더
3. 자기 cpp에서 다른 cpp는 헤더에 전방선언 후 cpp에서 헤더 include.

*/