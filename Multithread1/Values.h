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
#define MAX_CLIENT 20
using namespace std;

/*
1. ��� cpp�� ���� ���
2. �ڱ� cpp�� �ڱ� ���
3. �ڱ� cpp���� �ٸ� cpp�� ����� ���漱�� �� cpp���� ��� include.

*/