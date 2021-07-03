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
	//thread timeStampThread(UpdateTime);//생각해보니 업데이트할필요가 없네..?
}
PingManager::~PingManager()
{
}

void PingManager::TimeSynch_Receive(Player* player)
{
	long long timeNow = GetTimeNow();
	long long lastSentTime = pingRecords[player->actorNumber];
	long long elapsedTime = timeNow - lastSentTime;//갔다오는데 걸린시간
	long long oneWayPing = elapsedTime * 0.5; //가는데 걸릴시간
	pingRecords[player->actorNumber] = timeNow;
	cout << "Latency with player " << player->actorNumber << "\t " << oneWayPing <<"//server"<< serverTime << endl;
	TimeSync_Send(player, oneWayPing);//편도거리 0.5
}
/*

 ?		0		1		(2+1)=3		4
	init	             1받음
 0		1		2		3			4
			  1차받음
			  편도차이=1
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


