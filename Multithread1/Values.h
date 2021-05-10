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
1. 모든 cpp는 메인 헤더
2. 자기 cpp는 자기 헤더
3. 자기 cpp에서 다른 cpp는 헤더에 전방선언 후 cpp에서 헤더 include.

*/