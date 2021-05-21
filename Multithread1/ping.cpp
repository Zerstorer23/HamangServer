//
//	���۱�
//	http://www.sockaddr.com/ExampleSourceCode.html
//	http://cakel.tistory.com
//	�������� ������ �����Ӱ� ���Ǽ� �ֽ��ϴ�.
//
//	ping.cpp  -- ICMP �� RAW ������ ����ϴ� Ping ���α׷��Դϴ�.
//

#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include "ping.h"

#pragma comment(lib,"wsock32.lib")

// ���� �Լ�
void Ping(LPCSTR pstrHost);
void ReportError(LPCSTR pstrFrom);
int WaitForEchoReply(SOCKET s);
u_short in_cksum(u_short* addr, int Len);

// ICMP ���� �䱸/���� �Լ�
int SendEchoRequest(SOCKET, LPSOCKADDR_IN);
DWORD RecvEchoReply(SOCKET, LPSOCKADDR_IN, u_char*);

// ����
int main(int argc, char** argv)
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1);
	int nRet;

	// �μ� ��
	if (argc != 2)
	{
		fprintf(stderr, "\n���� : ping [ȣ��Ʈ��]\n");
		return 1;

	}

	// ���� ����
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet)
	{
		fprintf(stderr, "\n������ �����ϴµ� ���� �߻��߽��ϴ�.\n");
		return 1;

	}

	// ping �۾��� �մϴ�.
	Ping(argv[1]);

	// ������ ���ϴ�.
	WSACleanup();

	return 0;
}

// Ping() �Լ�
// SendEchoRequest() �Լ��� RecvEchoReply() �Լ��� �����ϰ�
// ����� ����մϴ�.

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

	// Raw ������ �����մϴ�.
	rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (rawSocket == SOCKET_ERROR)
	{
		ReportError("socket() �Լ� ���� �߻�");
		return;

	}

	// ȣ��Ʈ�� ã�� ���ϴ�(Lookup host)
	lpHost = gethostbyname(pstrHost);
	if (lpHost == NULL)
	{
		fprintf(stderr, "\nȣ��Ʈ�� ã������ �����ϴ� : %s\n", pstrHost);
		return;

	}

	// ������ ���� �ּҷ� ������ �õ� �մϴ�.
	saDest.sin_addr.s_addr = *((u_long FAR*)(lpHost->h_addr));
	saDest.sin_family = AF_INET;
	saDest.sin_port = 0;

	// ����ڿ��� ���α׷��� �ҷ��� �ϴ� �۾��� ǥ�� �մϴ�.
	printf("\n%s [%s] ȣ��Ʈ���� %d ����Ʈ�� �ڷḦ ���մϴ�.\n",
		pstrHost,
		inet_ntoa(saDest.sin_addr),
		REQ_DATASIZE);

	// ���(4) �� �۾��� �մϴ�.
	for (nLoop = 0; nLoop < 4; nLoop++)
	{
		// ICMP ���� �䱸(Echo request)�� �����ϴ�.
		SendEchoRequest(rawSocket, &saDest);

		// select() �Լ��� �ڷᰡ ������ ���� ��ٸ��ϴ�.
		nRet = WaitForEchoReply(rawSocket);
		if (nRet == SOCKET_ERROR)
		{
			ReportError("select() �Լ����� ���� �߻�");
			break;

		}

		if (!nRet)
		{
			printf("\n�ð� �ʰ�");
			break;
		}

		// ������ �޽��ϴ�.
		dwTimeSent = RecvEchoReply(rawSocket, &saSrc, &cTTL);

		// �ҿ�� �ð��� ����մϴ�.
		dwElapsed = GetTickCount() - dwTimeSent;
		printf("\n%s ���� ������ �޾ҽ��ϴ�. ����Ʈ=%d �ð� %ldms TTL=%d",
			inet_ntoa(saSrc.sin_addr), REQ_DATASIZE,
			dwElapsed, cTTL);
	}
	printf("\n");
	nRet = closesocket(rawSocket);
	if (nRet == SOCKET_ERROR)
		ReportError("closesocket() �Լ� ������ ���� �߻�.");

}

// SendEchoRequest()
// ���� �䱸 ��� ������ ä��� �������� �����ϴ�.
int SendEchoRequest(SOCKET s, LPSOCKADDR_IN lpstToAddr)
{
	static ECHOREQUEST echoReq;
	static int nId = 1;
	static int nSeq = 1;
	int nRet;

	// ���� �䱸 ������ ä��ϴ�.
	echoReq.icmpHdr.Type = ICMP_ECHOREQ;
	echoReq.icmpHdr.Code = 0;
	echoReq.icmpHdr.Checksum = 0;
	echoReq.icmpHdr.ID = nId++;
	echoReq.icmpHdr.Seq = nSeq++;

	// ���� ������ ä��ϴ�.
	for (nRet = 0; nRet < REQ_DATASIZE; nRet++)
		echoReq.cData[nRet] = ' ' + nRet;

	// ���� �� ƽ Ƚ��(tick count, �ð�) �����մϴ�.
	echoReq.dwTime = GetTickCount();

	// ���� �ڷḦ ��Ŷ���� �ְ� üũ��(�񱳰�)�� ����մϴ�.
	echoReq.icmpHdr.Checksum = in_cksum((u_short*)&echoReq, sizeof(ECHOREQUEST));

	// ���� �䱸 ��ȣ�� �����ϴ�.
	// sendto(����, ������ ������, �������� ����, ���� ��Ÿ���� �÷���, ���� ȣ��Ʈ �ּ�, ���� ȣ��Ʈ �ּ��� ũ��)
	// ������ ������ ������ ���� ���� �Ϲ������� �����ϴ� �Լ� �Դϴ�.
	// http://white.chungbuk.ac.kr/~jchern/sendto.html ����

	nRet = sendto(s,					// ����
		(LPSTR)&echoReq,		// ����
		sizeof(ECHOREQUEST),	// ���� ũ��
		0,						// �÷���
		(LPSOCKADDR)lpstToAddr,	// ������
		sizeof(SOCKADDR_IN));	// �ּ� ����

	if (nRet == SOCKET_ERROR)
		ReportError("sendto() �Լ� ������ ������ �߻��߽��ϴ�.");

	return (nRet);

}

// RecvEchoReply() �Լ�
// ������ �ڷḦ �ް� ���� �׸���� �м��մϴ�.
DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char* pTTL)
{
	ECHOREPLY echoReply;
	int nRet;
	int nAddrLen = sizeof(struct sockaddr_in);

	// ���� ������ �޽��ϴ�.
	// recvform(���� �ĺ���, ���ŵ� �����͸� ���� ����, ������ ����, ������ ��Ÿ���� �÷���, �� ���� �ּҸ�
	// ���� �ޱ� ���� ���� ������, ���� ����� ������ ������)
	// �������� ���� �����ͱ׷��� �ް�, �۽����� �ּҿ� ��Ʈ�� ��ȯ�ϴ� �Լ� �Դϴ�.
	// http://white.chungbuk.ac.kr/~jchern/recvfrom.html ����
	nRet = recvfrom(s,					// ����
		(LPSTR)&echoReply,		// ����
		sizeof(ECHOREPLY),		// ������ ũ��
		0,						// �÷���
		(LPSOCKADDR)lpsaFrom,	// �޴� �ּ�
		&nAddrLen);				// �ּ� ���Ǹ� ��� �ڷḦ ����Ű�� ������

// �ǵ��� ���� ���� �˻��մϴ�.
	if (nRet == SOCKET_ERROR)
		ReportError("recvfrom() �Լ� ������ ������ �߻��߽��ϴ�.");

	// ���� �ð� ���� IP TTL ���� �޽��ϴ�.
	*pTTL = echoReply.ipHdr.TTL;
	return(echoReply.echoRequest.dwTime);

}

// ReportError() �Լ�
// ���� �� �ֽ��ϱ�?
void ReportError(LPCSTR pWhere)
{
	fprintf(stderr, "\n%s WSAGetLastError ���� �ڵ� ��ȣ : %d\n", WSAGetLastError());
}

// WaitforEchoReply() �Լ�
// select() �Լ��� ����Ͽ� ���� �ڷᰡ ���� ���� �����մϴ�.
int WaitForEchoReply(SOCKET s)
{
	struct timeval Timeout;
	fd_set readfds;

	readfds.fd_count = 1;
	readfds.fd_array[0] = s;
	Timeout.tv_sec = 5;
	Timeout.tv_usec = 0;

	// select(I/O ��ȭ�� �����ϴ� ������ ����, �б� ���� ��ȭ ������ ����, ���� ���� ��ȭ ������ ����
	// ���� ���� ��ȭ ������ ����, ��ٸ��� ���� �ð�)
	// http://white.chungbuk.ac.kr/~jchern/select.html ����
	return(select(1, &readfds, NULL, NULL, &Timeout));

}

// ����ũ ������(Mike Muuss) �� in_cksum() �Լ�
// ���� ��(ping) �ڵ�� ���� �����Դϴ�.
// ���� http://myhome.shinbiro.com/~eaglelee/rawsocket.txt
//
//* ���� -
//* ����ũ ������(Mike Muuss)
//* �� ���� ź�� ������(U. S. Army Ballistic Research Laboratory)
//* 1983�� 12��
/*
 *		I N _ C K S U M
 *
 *
 *	���ͳ� �������� �迭 ����� ���� üũ�� ��� ��ƾ(C ����)
 *

*/

u_short in_cksum(u_short* addr, int len)
{
	register int nleft = len;
	register u_short* w = addr;
	register u_short answer;
	register int sum = 0;

	/*
	 * �츮�� ���� �˰����� �����մϴ�. 32 ��Ʈ �����(����)�� �Ἥ,
	 * �츮�� 16��Ʈ �ܾ ���������� ���� �� ��������,
	 * ���� 16��Ʈ�� �ڷ� ���� ���� 16��Ʈ�� �����ϴ�.
	 *
	 */

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;

	}

	/* �ʿ��ϴٸ�, Ȧ�� ����Ʈ �κ��� ���� �մϴ�. */
	if (nleft == 1) {
		u_short u = 0;
		*(u_char*)(&u) = *(u_char*)w;
		sum += u;

	}

	/*
	 *	���� 16��Ʈ�� ���� 16��Ʈ�� ���մϴ�.
	 */

	sum = (sum >> 16) + (sum & 0xffff); /* ���� 16��Ʈ�� ���� 16��Ʈ�� ���ϱ� */
	sum += (sum >> 16);					/* �ڸ� �ø����� ���մϴ�. */
	answer = ~sum;						/* 16 ��Ʈ�� �ڸ��ϴ�. */
	return (answer);

}