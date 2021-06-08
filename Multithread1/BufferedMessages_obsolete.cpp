
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
	rpc->message = message; //����Ʈ������ Ȱ�� TODO
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
}//������	
void BufferedMessages::RemoveViewID(int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->viewID == viewID) {
			delete* iter;
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	ReleaseMutex(hMutex);
}
void BufferedMessages::RemoveRPC(int playerNr, int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->viewID == viewID && (*iter)->playerActorNr == playerNr) {
			delete* iter;
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	ReleaseMutex(hMutex);

}
void BufferedMessages::RemoveAll() {
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		delete* iter;
		iter = messageQueue.erase(iter);
	}//for delete ��������clear
	//messageQueue.erase(remove(messageQueue.begin(), messageQueue.end(),NULL), messageQueue.end());
	ReleaseMutex(hMutex);
}
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
			if (message.length() + nextMessage.length() < BUFFER - 10) {//����
				message.append(nextMessage);
				iter++;
			}
		}
		LPPER_IO_DATA sendIO = serverInstance->CreateMessage(message);
		player->Send(sendIO);
	}
	ReleaseMutex(hMutex);
}
