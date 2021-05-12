#pragma once
#include "Player.h"
class PlayerManager
{

public:
	unordered_map<int, Player*> playerHash;
	int totalConnected;
	HANDLE hMutex;

public:
	PlayerManager() {
		totalConnected = 0;
		hMutex = CreateMutex(NULL, FALSE, NULL);
	}
	~PlayerManager() {
		for (pair<int, Player*> entry : playerHash) {
			SAFE_DELETE(entry.second)
		}
	}
	
	Player* CreatePlayer(LPPER_HANDLE_DATA handleInfo) {
		WaitForSingleObject(hMutex, INFINITE);
		Player* player = new Player();
		player->handleInfo = handleInfo;
		player->actorNumber = totalConnected;
		player->isHost = (totalConnected == 0);
		playerHash.insert(make_pair(player->actorNumber, player));
		handleInfo->player = player;
		totalConnected++;
		ReleaseMutex(hMutex);
		return player;
	}

	void RemovePlayer(int actorNumber) {
		WaitForSingleObject(hMutex, INFINITE);
		if (playerHash.count(actorNumber) != 1) {
			cout << "Missing actor" << endl;
			return;
		}
		closesocket(playerHash[actorNumber]->handleInfo->clientSocket);
		playerHash.erase(actorNumber);
		cout << "Removed actor " << actorNumber<<endl;
		ReleaseMutex(hMutex);
	}

	void BroadcastMessage(int & sourceActorNumber, LPPER_IO_DATA receivedIO, DWORD & bytesReceived) {
		for (auto entry : playerHash) {
			Player* targetPlayer = entry.second;
			if (targetPlayer->actorNumber == sourceActorNumber) continue;//자기자신은 제외
			targetPlayer->Send(receivedIO, bytesReceived);
		}
	}
	//void BroadcastMessageOthers(int whisperer, char* message, int amount) {
	//	for (pair<int,Player*> entry : playerHash){
	//		if (entry.second->actorNumber == whisperer) continue;
	//		entry.second->Send(message, amount);
	//	}
	//}


};

