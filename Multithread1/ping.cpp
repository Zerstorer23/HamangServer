//
//	저작권
//	http://www.sockaddr.com/ExampleSourceCode.html
//	http://cakel.tistory.com
//	교육용을 전제로 자유롭게 쓰실수 있습니다.
//
//	ping.cpp  -- ICMP 와 RAW 소켓을 사용하는 Ping 프로그램입니다.
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "ping.h"

#pragma comment(lib,"wsock32.lib")

// 내부 함수
void Ping(LPCSTR pstrHost);
void ReportError(LPCSTR pstrFrom);
int WaitForEchoReply(SOCKET s);
u_short in_cksum(u_short* addr, int Len);

// ICMP 에코 요구/응답 함수
int SendEchoRequest(SOCKET, LPSOCKADDR_IN);
DWORD RecvEchoReply(SOCKET, LPSOCKADDR_IN, u_char*);

// 시작
int main(int argc, char** argv)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1);
	int nRet;

	// 인수 비교
	if (argc != 2)
	{
		fprintf(stderr, "\n사용법 : ping [호스트명]\n");
		return 1;

	}

	// 윈속 시작
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet)
	{
		fprintf(stderr, "\n윈속을 시작하는데 오류 발생했습니다.\n");
		return 1;

	}

	// ping 작업을 합니다.
	Ping(argv[1]);

	// 윈속을 비웁니다.
	WSACleanup();

	return 0;
}

// Ping() 함수
// SendEchoRequest() 함수와 RecvEchoReply() 함수를 수행하고
// 결과를 출력합니다.

void Ping(LPCSTR pstrHost)
{
	SOCKET		rawSocket;
	LPHOSTENT	lpHost;
	struct		sockaddr_in saDest;
	struct		sockaddr_in saSrc;
	DWORD		dwTimeSent;
	DWORD		dwElapsed;
	u_char		cTTL;
	int			nLoop;
	int			nRet;

	// Raw 소켓을 생성합니다.
	rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (rawSocket == SOCKET_ERROR)
	{
		ReportError("socket() 함수 문제 발생");
		return;

	}

	// 호스트를 찾아 봅니다(Lookup host)
	lpHost = gethostbyname(pstrHost);
	if (lpHost == NULL)
	{
		fprintf(stderr, "\n호스트를 찾을수가 없습니다 : %s\n", pstrHost);
		return;

	}

	// 목적지 소켓 주소로 연결을 시도 합니다.
	saDest.sin_addr.s_addr = *((u_long FAR*)(lpHost->h_addr));
	saDest.sin_family = AF_INET;
	saDest.sin_port = 0;

	// 사용자에게 프로그램이 할려고 하는 작업을 표시 합니다.
	printf("\n%s [%s] 호스트에게 %d 바이트의 자료를 핑합니다.\n",
		pstrHost,
		inet_ntoa(saDest.sin_addr),
		REQ_DATASIZE);

	// 몇번(4) 핑 작업을 합니다.
	for (nLoop = 0; nLoop < 4; nLoop++)
	{
		// ICMP 에코 요구(Echo request)를 보냅니다.
		SendEchoRequest(rawSocket, &saDest);

		// select() 함수로 자료가 받을때 까지 기다립니다.
		nRet = WaitForEchoReply(rawSocket);
		if (nRet == SOCKET_ERROR)
		{
			ReportError("select() 함수에서 문제 발생");
			break;

		}

		if (!nRet)
		{
			printf("\n시간 초과");
			break;
		}

		// 응답을 받습니다.
		dwTimeSent = RecvEchoReply(rawSocket, &saSrc, &cTTL);

		// 소요된 시간을 계산합니다.
		dwElapsed = GetTickCount() - dwTimeSent;
		printf("\n%s 에서 응답을 받았습니다. 바이트=%d 시간 %ldms TTL=%d",
			inet_ntoa(saSrc.sin_addr), REQ_DATASIZE,
			dwElapsed, cTTL);
	}
	printf("\n");
	nRet = closesocket(rawSocket);
	if (nRet == SOCKET_ERROR)
		ReportError("closesocket() 함수 수행중 오류 발생.");

}

// SendEchoRequest()
// 에고 요구 헤더 정보를 채우고 목적지로 보냅니다.
int SendEchoRequest(SOCKET s, LPSOCKADDR_IN lpstToAddr)
{
	static ECHOREQUEST echoReq;
	static int nId = 1;
	static int nSeq = 1;
	int nRet;

	// 에코 요구 정보를 채웁니다.
	echoReq.icmpHdr.Type = ICMP_ECHOREQ;
	echoReq.icmpHdr.Code = 0;
	echoReq.icmpHdr.Checksum = 0;
	echoReq.icmpHdr.ID = nId++;
	echoReq.icmpHdr.Seq = nSeq++;

	// 보낼 정보를 채웁니다.
	for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
		echoReq.cData[nRet] = ' ' + nRet;

	// 보낼 때 틱 횟수(tick count, 시간) 저장합니다.
	echoReq.dwTime = GetTickCount();

	// 보낼 자료를 패킷에다 넣고 체크섬(비교값)을 계산합니다.
	echoReq.icmpHdr.Checksum = in_cksum((u_short*)&echoReq, sizeof(ECHOREQUEST));

	// 에코 요구 신호를 보냅니다.
	// sendto(소켓, 전송할 데이터, 데이터의 길이, 역할 나타내는 플래그, 원격 호스트 주소, 원격 호스트 주소의 크기)
	// 소켓이 접속의 유무와 관계 없이 일방적으로 전송하는 함수 입니다.
	// http://white.chungbuk.ac.kr/~jchern/sendto.html 참고

	nRet = sendto(s,					// 소켓
		(LPSTR)&echoReq,		// 버퍼
		sizeof(ECHOREQUEST),	// 버퍼 크기
		0,						// 플래그
		(LPSOCKADDR)lpstToAddr,	// 목적지
		sizeof(SOCKADDR_IN));	// 주소 길이

	if (nRet == SOCKET_ERROR)
		ReportError("sendto() 함수 수행중 문제가 발생했습니다.");

	return (nRet);

}

// RecvEchoReply() 함수
// 들어오는 자료를 받고 받은 항목들을 분석합니다.
DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char* pTTL)
{
	ECHOREPLY echoReply;
	int nRet;
	int nAddrLen = sizeof(struct sockaddr_in);

	// 에코 응답을 받습니다.
	// recvform(소켓 식별자, 수신된 데이터를 받을 버퍼, 버퍼의 길이, 역할을 나타내는 플래그, 준 곳의 주소를
	// 리턴 받기 위한 버퍼 포인터, 버퍼 사이즈를 지정한 포인터)
	// 소켓으로 부터 데이터그램을 받고, 송신자의 주소와 포트를 반환하는 함수 입니다.
	// http://white.chungbuk.ac.kr/~jchern/recvfrom.html 참고
	nRet = recvfrom(s,					// 소켓
		(LPSTR)&echoReply,		// 버퍼
		sizeof(ECHOREPLY),		// 버퍼의 크기
		0,						// 플래그
		(LPSOCKADDR)lpsaFrom,	// 받는 주소
		&nAddrLen);				// 주소 길의를 담는 자료를 가리키는 포인터

// 되돌아 오는 값을 검사합니다.
	if (nRet == SOCKET_ERROR)
		ReportError("recvfrom() 함수 수행중 문제가 발생했습니다.");

	// 보낸 시간 값과 IP TTL 값을 받습니다.
	*pTTL = echoReply.ipHdr.TTL;
	return(echoReply.echoRequest.dwTime);

}

// ReportError() 함수
// 무슨 일 있습니까?
void ReportError(LPCSTR pWhere)
{
	fprintf(stderr, "\n%s WSAGetLastError 오류 코드 번호 : %d\n", WSAGetLastError());
}

// WaitforEchoReply() 함수
// select() 함수를 사용하여 언제 자료가 읽혀 질지 결정합니다.
int WaitForEchoReply(SOCKET s)
{
	struct timeval Timeout;
	fd_set readfds;

	readfds.fd_count = 1;
	readfds.fd_array[0] = s;
	Timeout.tv_sec = 5;
	Timeout.tv_usec = 0;

	// select(I/O 변화를 감지하는 소켓의 갯수, 읽기 상태 변화 감지할 소켓, 쓰기 상태 변화 감지할 소켓
	// 예외 상태 변화 감지할 소켓, 기다리기 위한 시간)
	// http://white.chungbuk.ac.kr/~jchern/select.html 참고
	return(select(1, &readfds, NULL, NULL, &Timeout));

}

// 마이크 무유스(Mike Muuss) 의 in_cksum() 함수
// 원래 핑(ping) 코드와 그의 설명입니다.
// 참고 http://myhome.shinbiro.com/~eaglelee/rawsocket.txt
//
//* 저자 -
//* 마이크 무유스(Mike Muuss)
//* 미 육군 탄도 연구소(U. S. Army Ballistic Research Laboratory)
//* 1983년 12월
/*
 *		I N _ C K S U M
 *
 *
 *	인터넷 프로토콜 계열 헤더를 위한 체크섬 계산 루틴(C 버전)
 *

*/

u_short in_cksum(u_short* addr, int len)
{
	register int nleft = len;
	register u_short* w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 * 우리의 원래 알고리듬은 간단합니다. 32 비트 누산기(덧셈)를 써서,
	 * 우리는 16비트 단어를 순차적으로 더한 후 마지막에,
	 * 상위 16비트를 뒤로 접어 하위 16비트로 내립니다.
	 *
	 */

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;

	}

	/* 필요하다면, 홀수 바이트 부분을 정리 합니다. */
	if (nleft == 1) {
		u_short u = 0;
		*(u_char*)(&u) = *(u_char*)w;
		sum += u;

	}

	/*
	 *	상위 16비트를 하위 16비트로 더합니다.
	 */

	sum = (sum >> 16) + (sum & 0xffff); /* 상위 16비트를 하위 16비트로 더하기 */
	sum += (sum >> 16);					/* 자리 올림수를 더합니다. */
	answer = ~sum;						/* 16 비트로 자릅니다. */
	return (answer);

}