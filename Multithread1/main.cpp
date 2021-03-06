#include "IOCP_Server.h"
#include "PlayerManager.h"
#include "PingManager.h"
#include "BufferedMessages.h"
#include "DBMS_NI.h"
#include <io.h>
#include <fcntl.h>
#include <io.h>
#include <fcntl.h>
#include <locale.h>

int main()
{
    cout.imbue(locale("korean"));
    SetConsoleOutputCP(65001);

    ;
    /*
    * 6월 26일 글자가 utf8과 그냥 string일떄 바이트 정보가 달랏음 -> legnth가 다름
    1.utf8로 들어온 코드가 split이 잘 안됨
    2.cpp 내부적으로 인코딩이 무엇인지 알수없음
    3. unicode로 확정짓고 작업
    4. utf-8로 변환후 전송

    3대 쟁점
    1.split이 잘되나
    2. append가 잘되나
    3. stoi, to_string가 잘되나 
    4. unicode가 필요할때는 글자수를 셀 때
    
    결론은..string 인쾯ㅇ은 파일인코딩 따락ㅁ. 파일 인코딩 utf8로 저장
    u8"strug"이용

    */
    //for (int i = 0; i < strlen(u16s.c_str()); i++) {
    //    cout << u8s.c_str()[i]<<" vs " << u16s.c_str()[i] << endl;
    //}
    //for (int i = 0; i < unknown.size(); i++) {
    //    cout << unknown.c_str()[i] << endl;
    //}
    cout << u8"IP 주소 입력"s << endl;
    char iptemp[256];
  //  cin >> iptemp;
    string ip = "172.30.1.27";// "127.0.0.1";
    //iptemp;
    string port = "9000";
    IOCP_Server * myServer =IOCP_Server::GetInst();
    PlayerManager* playerManager = PlayerManager::GetInst();
    PingManager* pingManager = PingManager::GetInst();
    DBMS_NI* dbManager = DBMS_NI::GetInst();
    dbManager->OpenDatabase();

    dbManager->ShowAll();
    return 0;
    BufferedMessages* messageBuffer = BufferedMessages::GetInst();
    myServer->InitialiseServer(ip, port);
    myServer->OpenServer();
    myServer->CloseServer();
    IOCP_Server::DestroyInst();
}