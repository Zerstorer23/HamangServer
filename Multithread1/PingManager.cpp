#include "PingManager.h"
#include "NetworkMessage.h"
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

void PingManager::TimeSynch_Receive(Player* player, int requestBufferedRPCs)
{
	UpdateTime();
	P_PingRecord ping = pingRecords[player->actorNumber];
	assert(!ping->received);
	ping->elapsedTime = serverTime - ping->lastSentTime;
	ping->received = true;
	cout << "Latency with player " << player->actorNumber << "\t " << (ping->elapsedTime / 2)<<"//server"<< serverTime << endl;
	PushServerTimeToPlayer(player, 1, ping->elapsedTime / 2, requestBufferedRPCs);//편도거리 0.5
	SAFE_DELETE(ping);
}


void PingManager::Handle_Request_TimeSynch(Player * targetPlayer, int isModification, int requestBufferedRPCs)
{
	UpdateTime();
	if (isModification == 0) {
		PushServerTimeToPlayer(targetPlayer, 0, serverTime, requestBufferedRPCs);
	}
	else {
		TimeSynch_Receive(targetPlayer, requestBufferedRPCs);

	}
}

void PingManager::PushServerTimeToPlayer(Player * player, int isModification, long long timeValue, int requestBufferedRPCs)
{
	//LEX / 0 =SERVER / PING=MESSAGEINFO / targetPlater /1 OR 0 = INDEX TO REFER / SERVERTIME or EXPECTEDDELAY
	NetworkMessage netMessage;
	netMessage.Append("0");
	netMessage.Append(to_string((int)MessageInfo::ServerCallbacks));
	netMessage.Append(to_string((int)LexCallback::PushServerTime));
	netMessage.Append(to_string(player->actorNumber));
	netMessage.Append(to_string(isModification));
	netMessage.Append(to_string(timeValue));
	netMessage.Append(to_string(requestBufferedRPCs));
	string message = netMessage.BuildNewSignedMessage();
	DWORD bytesSent =(DWORD) message.length();
	player->Send((char*)message.c_str(), bytesSent);
	RecordPing_Send(player->actorNumber, timeValue);
	cout << "Push Time" << timeValue << endl;
}


