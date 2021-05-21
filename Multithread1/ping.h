// ���۱�
// http://www.sockaddr.com/ExampleSourceCode.html
// http://cakel.tistory.com
// �������� ������ �����Ӱ� ���Ǽ� �ֽ��ϴ�.
// 
// ping.h - RFC �԰ݿ� �°� ICMP �� IP ����� �����ϴ� �����Դϴ�.
// 


// packing �뷮 ���� �����ϴ� �κ��Դϴ�. alignment �� 1 �� �ξ �ּ� �ڷ� ������ �븳�ϴ�.
// ��ſ� �����ϴ� �ӵ��� �������Ƿ� ���� ������� �ƴմϴ�.
#pragma pack(1)

#define ICMP_ECHOREPLAY 0
#define ICMP_ECHOREQ	8

// IP ��� �κ� -- RFC 791
// http://www.faqs.org/rfcs/rfc791.html
// http://blog.naver.com/after1007/50014330586 ����

typedef struct tagIPHDR
{
	u_char	VIHL;			// ������ IHL(Internet Header Length) ��
	u_char	TOS;			// Type of Service (������ ����)
	short	TotLen;			// Total Length (��ü ����)
	short	ID;				// Identification (�ĺ���)
	short	FlagOff;		// Fragment Offset (���� ��ǥ)
	u_char	TTL;			// Time to Live (���� �ð�)
	u_char	Protocol;		// Protocol (��������, ���� �Ծ�)
	u_short	Checksum;		// üũ��(�� ��)
	struct	in_addr iaSrc;	// Internet Address - Source (���ͳ� �ּ� - ����)
	struct	in_addr	iaDsc;	// Internet Address - Destination (���ͳ� �ּ� - ���)

}	IPHDR, * PIPHDR;

// ICMP ��� -- RFC 792
// http://www.faqs.org/rfcs/rfc792.html
// http://www.microsoft.com/korea/technet/deploy/tcpintro5.asp
// http://technet2.microsoft.com/WindowsServer/ko/Library/732438fe-70c5-4e68-9663-ecbd955d29ea1042.mspx?mfr=true
// http://blog.naver.com/isnta/20024953151 ����

typedef struct tagICMPHDR
{
	u_char	Type;			// ����
	u_char	Code;			// �ڵ�
	u_short	Checksum;		// üũ��
	u_short	ID;				// �ĺ���
	u_short	Seq;			// ����
	char	Data;			// �ڷ�(������)

}	ICMPHDR, * PICMPHDR;

#define REQ_DATASIZE 32		// ����(Echo) �䱸 �ڷ� ����

// ICMP Echo Request - ���� �ڷ� �䱸
typedef struct tagECHOREQUEST
{
	ICMPHDR	icmpHdr;
	DWORD	dwTime;
	char	cData[REQ_DATASIZE];
}	ECHOREQUEST, * PECHOREQUEST;

// ICMP Echo Reply			// ���� ����
typedef struct tagECHOREPLY
{
	IPHDR		ipHdr;
	ECHOREQUEST	echoRequest;
	char		cFiller[256];
}	ECHOREPLY, * PECHOREPLY;

#pragma pack()		// packing �뷮 ���� �⺻������ �����մϴ�.