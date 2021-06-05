#include "BufferedMessages.h"
#include "NetworkMessage.h"
#include "IOCP_Server.h"
#include "Player.h"
DEFINITION_SINGLE(BufferedMessages)

BufferedMessages::BufferedMessages() {
	hMutex = CreateMutex(NULL, FALSE, NULL);
}
BufferedMessages::~BufferedMessages() {
	for (auto message : messageQueue) {
		SAFE_DELETE(message);
	}
}
void BufferedMessages::EnqueueMessage(int playerNr, int viewID, string message) {
	WaitForSingleObject(hMutex, INFINITE);
	PRPC rpc = new RPC();
	rpc->playerActorNr = playerNr;
	rpc->viewID = viewID;
	rpc->message = message; //스마트포인터 활용 TODO
	messageQueue.push_back(rpc);
	ReleaseMutex(hMutex);
}
void BufferedMessages::RemovePlayerNr(int playerNr) {
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->playerActorNr == playerNr) {
			delete* iter;
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	ReleaseMutex(hMutex);
}//게임잡	
void BufferedMessages::SendBufferedMessages(Player* player)
{
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	IOCP_Server* serverInstance = IOCP_Server::GetInst();
	while (iter != iterEnd) {
		string message = (*iter)->message;
		iter++;
		while (iter != iterEnd) {
			string nextMessage = (*iter)->message;
			if (message.length() + nextMessage.length() < BUFFER - 10) {//여유
				message.append(nextMessage);
				iter++;
			}
		}
		LPPER_IO_DATA sendIO = serverInstance->CreateMessage(message);
		player->Send(sendIO);
	}
	ReleaseMutex(hMutex);
}
