#pragma once

#include <iostream>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <process.h>
#include <cassert>
#include <unordered_map>
#include <string.h>

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
	OVERLAPPED overlapped;//����ü�� �ּҴ� ù���� overlap���ּ�
	WSABUF wsaBuf;
	char buffer[BUFFER];
	int rwMode;

}PER_IO_DATA, * LPPER_IO_DATA;

/*
1. ��� cpp�� ���� ���
2. �ڱ� cpp�� �ڱ� ���
3. �ڱ� cpp���� �ٸ� cpp�� ����� ���漱�� �� cpp���� ��� include.

*/