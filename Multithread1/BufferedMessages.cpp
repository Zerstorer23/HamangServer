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
		message.reset();
	}
}
void BufferedMessages::EnqueueMessage(int playerNr, int viewID, string message) {
	WaitForSingleObject(hMutex, INFINITE);
	shared_ptr<RPC>  rpc(new RPC());
	rpc->playerActorNr = playerNr;
	rpc->viewID = viewID;
	rpc->message = message; 
	//auto a = rpc; // 카운트2

	messageQueue.push_back(rpc);
//unique는 user count 1 이상 불가..

	ReleaseMutex(hMutex);
}
void BufferedMessages::RemovePlayerNr(int playerNr) {
	WaitForSingleObject(hMutex, INFINITE);
	//cout << "플레이어"<<playerNr<<" 삭제시작 " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	int i = 0;
	while (iter != iterEnd) {
		if ((*iter)->playerActorNr == playerNr) {
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	//cout << "잔여 RPC크기 " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}//게임잡	
void BufferedMessages::RemoveViewID(int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	//cout << "View"<<viewID<<" 삭제시작 " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	int i = 0;
	while (iter != iterEnd) {
		if ((*iter)->viewID == viewID) {
	//		cout <<  L"\t삭제:" <<(*iter)->message << endl;
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	//cout << "잔여 RPC크기 " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}
void BufferedMessages::RemoveRPC(int playerNr, int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	//cout << "player viewID 삭제시작 " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->viewID == viewID && (*iter)->playerActorNr == playerNr) {
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	cout << u8"RPC크기 " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);

}
void BufferedMessages::RemoveAll() {
	WaitForSingleObject(hMutex, INFINITE);
	cout << u8"전체삭제 시작 " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		//cout << iter->use_count() << endl;
		iter->reset();
		iter = messageQueue.erase(iter);
	}//for delete 마지막에clear
	//messageQueue.erase(remove(messageQueue.begin(), messageQueue.end(),NULL), messageQueue.end());
	cout <<u8"RPC크기 "<< messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}

void BufferedMessages::SendBufferedMessages(Player* player)
{
	player->isConnected = true;
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
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
		//cout << "Buffered RPC " << message << endl;
		player->Send(message, true);
	}
	ReleaseMutex(hMutex);
}
