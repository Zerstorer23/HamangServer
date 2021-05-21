#include "PingManager.h"
#include "NetworkMessage.h"
#include "IOCP_Server.h"
#include "Player.h"
void PingManager::TestPingStatus(unordered_map<int, Player*> players)
{
	for (auto entry : players) {
		PingPlayer(entry.second);
	}
}

void PingManager::PingPlayer(Player * player)
{
	long timeNow = CurrentTimeInMills();
	NetworkMessage netMessage;
	netMessage.Append("0");
	netMessage.Append(to_string((int)LexCallback::Ping));
	netMessage.Append(to_string(player->actorNumber));
	netMessage.Append(to_string(timeNow));
	string message = netMessage.BuildNewSignedMessage();
	DWORD bytesSent = message.length() + 1;
	player->Send((char*)message.c_str(), bytesSent);
	RecordPing_Send(player->actorNumber, timeNow);
}
