// 저작권
// http://www.sockaddr.com/ExampleSourceCode.html
// http://cakel.tistory.com
// 교육용을 전제로 자유롭게 쓰실수 있습니다.
// 
// ping.h - RFC 규격에 맞게 ICMP 와 IP 헤더를 지정하는 파일입니다.
// 


// packing 용량 단위 설정하는 부분입니다. alignment 를 1 로 두어서 최소 자료 전송을 노립니다.
// 대신에 접근하는 속도가 떨어지므로 좋은 방법만은 아닙니다.
#pragma pack(1)

#define ICMP_ECHOREPLAY 0
#define ICMP_ECHOREQ	8

// IP 헤더 부분 -- RFC 791
// http://www.faqs.org/rfcs/rfc791.html
// http://blog.naver.com/after1007/50014330586 참고

typedef struct tagIPHDR
{
	u_char	VIHL;			// 버전과 IHL(Internet Header Length) 값
	u_char	TOS;			// Type of Service (서비스의 형태)
	short	TotLen;			// Total Length (전체 길이)
	short	ID;				// Identification (식별자)
	short	FlagOff;		// Fragment Offset (단편 좌표)
	u_char	TTL;			// Time to Live (생존 시간)
	u_char	Protocol;		// Protocol (프토토콜, 전송 규약)
	u_short	Checksum;		// 체크섬(값 비교)
	struct	in_addr iaSrc;	// Internet Address - Source (인터넷 주소 - 원본)
	struct	in_addr	iaDsc;	// Internet Address - Destination (인터넷 주소 - 대상)

}	IPHDR, * PIPHDR;

// ICMP 헤더 -- RFC 792
// http://www.faqs.org/rfcs/rfc792.html
// http://www.microsoft.com/korea/technet/deploy/tcpintro5.asp
// http://technet2.microsoft.com/WindowsServer/ko/Library/732438fe-70c5-4e68-9663-ecbd955d29ea1042.mspx?mfr=true
// http://blog.naver.com/isnta/20024953151 참고

typedef struct tagICMPHDR
{
	u_char	Type;			// 형식
	u_char	Code;			// 코드
	u_short	Checksum;		// 체크섬
	u_short	ID;				// 식별자
	u_short	Seq;			// 순서
	char	Data;			// 자료(데이터)

}	ICMPHDR, * PICMPHDR;

#define REQ_DATASIZE 32		// 에코(Echo) 요구 자료 길이

// ICMP Echo Request - 에코 자료 요구
typedef struct tagECHOREQUEST
{
	ICMPHDR	icmpHdr;
	DWORD	dwTime;
	char	cData[REQ_DATASIZE];
}	ECHOREQUEST, * PECHOREQUEST;

// ICMP Echo Reply			// 에코 응답
typedef struct tagECHOREPLY
{
	IPHDR		ipHdr;
	ECHOREQUEST	echoRequest;
	char		cFiller[256];
}	ECHOREPLY, * PECHOREPLY;

#pragma pack()		// packing 용량 단위 기본값으로 변경합니다.