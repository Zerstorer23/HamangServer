#pragma once
#include "Values.h"
class Player;
class NetworkMessage;
class PlayerManager
{

public:
	unordered_map<int, Player*> playerHash;
	Player* masterPlayer;
	int nextActorNumber;
	HANDLE hMutex;
	DECLARE_SINGLE(PlayerManager)

public:
	
	Player* CreatePlayer(LPPER_HANDLE_DATA handleInfo) ;

	void RemovePlayer(int actorNumber);

	void BroadcastMessage(int& sourceActorNumber, string& message);
	void BroadcastMessageAll(string& message);
	void PrintPlayers();
	void SetMasterClient(int newMaster);
	void EncodePlayersToNetwork(Player* joinedPlayer, NetworkMessage& netMessage);

	void ChangeMasterClientOnDisconnect();
	Player* GetFirstPlayer() {
		return playerHash.begin()->second;
	}

	int GetPlayerCount() {
		return playerHash.size();
	}
	void Reset() {
		masterPlayer = nullptr;
		nextActorNumber = 1;
	}
};

