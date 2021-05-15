#pragma once
#include "Player.h"
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
	~PlayerManager() {
		for (pair<int, Player*> entry : playerHash) {
			SAFE_DELETE(entry.second)
		}
	}
	
	Player* CreatePlayer(LPPER_HANDLE_DATA handleInfo) {
		WaitForSingleObject(hMutex, INFINITE);
		Player* player = new Player();
		player->handleInfo = handleInfo;
		player->actorNumber = nextActorNumber;
		player->isMasterClient = (playerHash.size() == 0);
		playerHash.insert(make_pair(player->actorNumber, player));
		handleInfo->player = player;
		nextActorNumber++;
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

	void BroadcastMessage(int & sourceActorNumber, char * sendBuffer, DWORD & bytesReceived) {
		for (auto entry : playerHash) {
			Player* targetPlayer = entry.second;
			if (targetPlayer->actorNumber == sourceActorNumber) continue;//자기자신은 제외
			targetPlayer->Send(sendBuffer, bytesReceived);
		}
	}
	void BroadcastMessageAll(char* sendBuffer, DWORD& bytesReceived) {
		for (auto entry : playerHash) {
			entry.second->Send(sendBuffer, bytesReceived);
		}
	}
	void PrintPlayers() {
		cout << "Connected players :" << playerHash.size() << endl;
		for (auto entry : playerHash) {
			cout << entry.second->actorNumber << endl;
		}
	}
	string EncodePlayersToNetwork(Player * joinedPlayer) {
		string message = NET_DELIM;
		message = message.append(to_string(playerHash.size())).append(joinedPlayer->EncodeToNetwork());
		for (auto entry : playerHash) {
			if (entry.first == joinedPlayer->actorNumber) continue;
			message = message.append(entry.second->EncodeToNetwork());
		}
		cout << "PLayer code: " << message << endl;
		return message;	
	}

	//void BroadcastMessageOthers(int whisperer, char* message, int amount) {
	//	for (pair<int,Player*> entry : playerHash){
	//		if (entry.second->actorNumber == whisperer) continue;
	//		entry.second->Send(message, amount);
	//	}
	//}


};

