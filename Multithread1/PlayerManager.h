#pragma once
#include "Player.h"
class PlayerManager
{

public:
	unordered_map<int, Player*>  playerHash;
	int totalConnected;
	HANDLE hMutex;

public:
	PlayerManager() {
		totalConnected = 0;
		hMutex = CreateMutex(NULL, FALSE, NULL);
	}
	~PlayerManager() {
		for (pair<int, Player*> entry : playerHash) {
			delete entry.second;
			entry.second = nullptr;
		}
		
	}
	
	Player* CreatePlayer(SOCKET& clientSocket) {
		WaitForSingleObject(hMutex, INFINITE);
		Player* player = new Player();
		player->clientSocket = clientSocket;
		player->actorNumber = totalConnected;
		player->isHost = (totalConnected == 0);
		playerHash.insert(make_pair(player->actorNumber, player));
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
		closesocket(playerHash[actorNumber]->clientSocket);
		playerHash.erase(actorNumber);
		cout << "Removed actor " << actorNumber<<endl;
		ReleaseMutex(hMutex);
	}

	void BroadcastMessageOthers(int whisperer, char* message, int amount) {
		for (pair<int,Player*> entry : playerHash){
			if (entry.second->actorNumber == whisperer) continue;
			entry.second->Send(message, amount);
		}
	}


};

