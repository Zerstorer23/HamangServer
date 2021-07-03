#include "PingManager.h"
#include "NetworkMessage.h"
#include "BufferedMessages.h"
#include "PlayerManager.h"
#include "IOCP_Server.h"
#include "Player.h"

PingManager* PingManager::instance = NULL;

PingManager::PingManager()
{
	UpdateTime();
	cout << "Server Time" << serverTime << endl;
	//thread timeStampThread(UpdateTime);//�����غ��� ������Ʈ���ʿ䰡 ����..?
}
PingManager::~PingManager()
{
}

void PingManager::TimeSynch_Receive(Player* player)
{
	long long timeNow = GetTimeNow();
	long long lastSentTime = pingRecords[player->actorNumber];
	long long elapsedTime = timeNow - lastSentTime;//���ٿ��µ� �ɸ��ð�
	long long oneWayPing = elapsedTime * 0.5; //���µ� �ɸ��ð�
	pingRecords[player->actorNumber] = timeNow;
	cout << "Latency with player " << player->actorNumber << "\t " << oneWayPing <<"//server"<< serverTime << endl;
	TimeSync_Send(player, oneWayPing);//���Ÿ� 0.5
}
/*

 ?		0		1		(2+1)=3		4
	init	             1����
 0		1		2		3			4
			  1������
			  ������=1
*/

void PingManager::TimeSync_Send(Player * player, long long timeValue)
{
	//LEX / 0 =SERVER / PING=MESSAGEINFO / targetPlater / modifiedTime
	NetworkMessage netMessage;
	netMessage.Append(0);
	netMessage.Append(((int)MessageInfo::ServerCallbacks));
	netMessage.Append(((int)LexCallback::ModifyServerTime));
	netMessage.Append((player->actorNumber));
	netMessage.Append(to_string(timeValue));
	string message = netMessage.BuildNewSignedMessage();
	//DWORD bytesSent =(DWORD) message.length();
	player->Send(message,true);
	cout << "Push Time" << timeValue << endl;

}


