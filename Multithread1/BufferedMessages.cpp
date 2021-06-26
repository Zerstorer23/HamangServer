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
void BufferedMessages::EnqueueMessage(int playerNr, int viewID, wstring message) {
	WaitForSingleObject(hMutex, INFINITE);
	shared_ptr<RPC>  rpc(new RPC());
	rpc->playerActorNr = playerNr;
	rpc->viewID = viewID;
	rpc->message = message; //스마트포인터 활용 TODO
	//auto a = rpc; // 카운트2

	messageQueue.push_back(rpc);
//unique는 user count 1 이상 불가..

	ReleaseMutex(hMutex);
}
void BufferedMessages::RemovePlayerNr(int playerNr) {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "playerNr 삭제시작 " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->playerActorNr == playerNr) {
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	cout << "RPC크기 " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}//게임잡	
void BufferedMessages::RemoveViewID(int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "viewID 삭제시작 " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->viewID == viewID) {
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	cout << "RPC크기 " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}
void BufferedMessages::RemoveRPC(int playerNr, int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "player viewID 삭제시작 " << messageQueue.size() << endl;
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
	cout << "RPC크기 " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);

}
void BufferedMessages::RemoveAll() {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "전체삭제 시작 " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		//cout << iter->use_count() << endl;
		iter->reset();
		iter = messageQueue.erase(iter);
	}//for delete 마지막에clear
	//messageQueue.erase(remove(messageQueue.begin(), messageQueue.end(),NULL), messageQueue.end());
	cout <<"RPC크기 "<< messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}
void BufferedMessages::SendBufferedMessages(Player* player)
{
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	IOCP_Server* serverInstance = IOCP_Server::GetInst();
	while (iter != iterEnd) {
		wstring message = (*iter)->message;
		iter++;
		while (iter != iterEnd) {
			wstring nextMessage = (*iter)->message;
			if (message.length() + nextMessage.length() < BUFFER - 10) {//여유
				message.append(nextMessage);
				iter++;
			}
		}
		wcout << "Buffered RPC " << message << endl;
		player->Send(message);
	}
	//TODO Buffered rpc 안보내짐
	ReleaseMutex(hMutex);
}
