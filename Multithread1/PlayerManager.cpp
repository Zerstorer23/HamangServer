#include "PlayerManager.h"
#include "Player.h"
#include "NetworkMessage.h"
#include "IOCP_Server.h"
DEFINITION_SINGLE(PlayerManager)
PlayerManager::PlayerManager() {
		masterPlayer = nullptr;
		nextActorNumber = 1;
		hMutex = CreateMutex(NULL, FALSE, NULL);
	}
PlayerManager::~PlayerManager()
{
	for (pair<int, Player*> entry : playerHash) {
		SAFE_DELETE(entry.second)
	}
}

Player* PlayerManager::CreatePlayer(LPPER_HANDLE_DATA handleInfo)
{
	WaitForSingleObject(hMutex, INFINITE);
	Player* player = new Player();
	player->handleInfo = handleInfo;
	player->SetActorNumber(nextActorNumber);
	player->isMasterClient = (playerHash.size() == 0);
	if (player->isMasterClient) {
		masterPlayer = player;
	}
	playerHash.insert(make_pair(player->actorNumber, player));
	handleInfo->player = player;
	nextActorNumber++;
	ReleaseMutex(hMutex);
	return player;
}

void PlayerManager::SetMasterClient(int newMaster) {
	WaitForSingleObject(hMutex, INFINITE);
	if (masterPlayer) {
		masterPlayer->isMasterClient = false;
	}
	playerHash[newMaster]->isMasterClient = true;
	masterPlayer = playerHash[newMaster];
	ReleaseMutex(hMutex);
};
void PlayerManager::ChangeMasterClientOnDisconnect() {
	WaitForSingleObject(hMutex, INFINITE);
	masterPlayer = GetFirstPlayer();
	masterPlayer->isMasterClient = true;   
	
	NetworkMessage eolMessage;
	eolMessage.Append(L"0");
	eolMessage.Append(to_wstring((int)MessageInfo::ServerCallbacks));
	eolMessage.Append(to_wstring((int)LexCallback::MasterClientChanged));
	eolMessage.Append(to_wstring(masterPlayer->actorNumber));
	wstring message = eolMessage.BuildNewSignedMessage();
	//DWORD size = message.length();
	//LPPER_IO_DATA sendIO = IOCP_Server::GetInst()->CreateMessage(message);
	cout << "Changing master on disconnect to"<<masterPlayer->actorNumber << endl;
	PlayerManager::GetInst()->BroadcastMessageAll(message);
	ReleaseMutex(hMutex);
};

void PlayerManager::RemovePlayer(int actorNumber)
{
	WaitForSingleObject(hMutex, INFINITE);
	if (playerHash.count(actorNumber) != 1) {
		cout << "Missing actor" << endl;
		ReleaseMutex(hMutex);
		return;
	}
	closesocket(playerHash[actorNumber]->handleInfo->clientSocket);
	playerHash.erase(actorNumber);
	cout << "Removed actor " << actorNumber << endl;
	ReleaseMutex(hMutex);
}
/*
void PlayerManager::BroadcastMessage(int& sourceActorNumber, char* sendBuffer, DWORD& bytesReceived)
{
	for (auto entry : playerHash) {
		Player* targetPlayer = entry.second;
		if (targetPlayer->actorNumber == sourceActorNumber) continue;//자기자신은 제외

		targetPlayer->Send(sendBuffer, bytesReceived);
	}
}*/
void PlayerManager::BroadcastMessage(int& sourceActorNumber,wstring & message)
{
	for (auto entry : playerHash) {
		Player* targetPlayer = entry.second;
		if (targetPlayer->actorNumber == sourceActorNumber) continue;//자기자신은 제외

		targetPlayer->Send(message,false);
	}
}
void PlayerManager::BroadcastMessageAll(wstring& message)
{
	for (auto entry : playerHash) {
		entry.second->Send(message,false);
	}
}

void PlayerManager::PrintPlayers()
{
	cout << "Connected players :" << playerHash.size() << endl;
	for (auto entry : playerHash) {
		cout << entry.second->actorNumber << endl;
	}
}

void PlayerManager::EncodePlayersToNetwork(Player* joinedPlayer, NetworkMessage& netMessage) {
	netMessage.Append(to_wstring(playerHash.size()));
	joinedPlayer->EncodeToNetwork(netMessage);
	for (auto entry : playerHash) {
		if (entry.first == joinedPlayer->actorNumber) continue;
		entry.second->EncodeToNetwork(netMessage);
	}
}