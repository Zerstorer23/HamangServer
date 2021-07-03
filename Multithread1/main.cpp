#include "IOCP_Server.h"
#include "PlayerManager.h"
#include "PingManager.h"
#include "BufferedMessages.h"
#include <io.h>
#include <fcntl.h>
#include <io.h>
#include <fcntl.h>
#include <locale.h>

int main()
{
    cout.imbue(locale("korean"));
    SetConsoleOutputCP(65001);
   // cout.imbue(locale("korean"));

    string u8s = u8"abc������"s;
    cout << u8s.length() << endl;
    auto u16s = u"abc1������"s;
//    string unknown = u8s + u16s;
    wstring unis = L"abc������";

    ;
    /*
    * 6�� 26�� ���ڰ� utf8�� �׳� string�ϋ� ����Ʈ ������ �޶��� -> legnth�� �ٸ�
    1.utf8�� ���� �ڵ尡 split�� �� �ȵ�
    2.cpp ���������� ���ڵ��� �������� �˼�����
    3. unicode�� Ȯ������ �۾�
    4. utf-8�� ��ȯ�� ����

    3�� ����
    1.split�� �ߵǳ�
    2. append�� �ߵǳ�
    3. stoi, to_string�� �ߵǳ� 
    4. unicode�� �ʿ��Ҷ��� ���ڼ��� �� ��
    
    �����..string �β����� �������ڵ� ������. ���� ���ڵ� utf8�� ����
    u8"strug"�̿�

    */
    //for (int i = 0; i < strlen(u16s.c_str()); i++) {
    //    cout << u8s.c_str()[i]<<" vs " << u16s.c_str()[i] << endl;
    //}
    //for (int i = 0; i < unknown.size(); i++) {
    //    cout << unknown.c_str()[i] << endl;
    //}

    string ip = "172.30.1.27";// "127.0.0.1";
    string port = "9000";
    IOCP_Server * myServer =IOCP_Server::GetInst();
    PlayerManager* playerManager = PlayerManager::GetInst();
    PingManager* pingManager = PingManager::GetInst();
    BufferedMessages* messageBuffer = BufferedMessages::GetInst();
    myServer->InitialiseServer(ip, port);
    myServer->OpenServer();
    myServer->CloseServer();
    IOCP_Server::DestroyInst();
}