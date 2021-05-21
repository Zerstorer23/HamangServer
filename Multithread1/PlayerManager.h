#pragma once
#include "Values.h"
class Player;
class NetworkMessage;
class PlayerManager
{

public:
	unordered_map<int, Player*> playerHash;
	int nextActorNumber;
	HANDLE hMutex;

public:
	PlayerManager() {
		nextActorNumber = 1;
		hMutex = CreateMutex(NULL, FALSE, NULL);
	}
	~PlayerManager();
	
	Player* CreatePlayer(LPPER_HANDLE_DATA handleInfo) ;

	void RemovePlayer(int actorNumber);

	void BroadcastMessage(int& sourceActorNumber, char* sendBuffer, DWORD& bytesReceived);
	void BroadcastMessageAll(char* sendBuffer, DWORD& bytesReceived);
	void PrintPlayers();

	void EncodePlayersToNetwork(Player* joinedPlayer, NetworkMessage& netMessage);
	
};

