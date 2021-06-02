#pragma once
#include "Values.h"

class Player;
class IOCP_Server;
class NeworkMessage;
class BufferedMessages
{
private:
	vector<PRPC> messageQueue;
public:
	HANDLE hMutex;
	DECLARE_SINGLE(BufferedMessages)

public:
	void EnqueueMessage(int playerNr, int viewID, string message) {
		WaitForSingleObject(hMutex, INFINITE);
		PRPC rpc = new RPC();
		rpc->playerActorNr = playerNr;
		rpc->viewID = viewID;
		rpc->message = message;
		messageQueue.push_back(rpc);
		ReleaseMutex(hMutex);
	}
	

	void RemovePlayerNr(int playerNr) {
		WaitForSingleObject(hMutex, INFINITE);
		auto iter = messageQueue.begin();
		auto iterEnd = messageQueue.end();
		while (iter != iterEnd) {
			if ((*iter)->playerActorNr == playerNr) {
				SAFE_DELETE(*iter);
				iter = messageQueue.erase(iter);
			}
			else {
				iter++;
			}
		}
		ReleaseMutex(hMutex);
	}
	void RemoveViewID(int viewID) {
		WaitForSingleObject(hMutex, INFINITE);
		auto iter = messageQueue.begin();
		auto iterEnd = messageQueue.end();
		while (iter != iterEnd) {
			if ((*iter)->viewID == viewID) {
				SAFE_DELETE(*iter);
				iter = messageQueue.erase(iter);
			}
			else {
				iter++;
			}
		}
		ReleaseMutex(hMutex);
	}
	void RemoveRPC(int playerNr, int viewID) {
		WaitForSingleObject(hMutex, INFINITE);
		auto iter = messageQueue.begin();
		auto iterEnd = messageQueue.end();
		while (iter != iterEnd) {
			if ((*iter)->viewID == viewID && (*iter)->playerActorNr == playerNr) {
				SAFE_DELETE(*iter);
				iter = messageQueue.erase(iter);
			}
			else {
				iter++;
			}
		}
		ReleaseMutex(hMutex);
	
	}
	void RemoveAll() {
		WaitForSingleObject(hMutex, INFINITE);
		auto iter = messageQueue.begin();
		auto iterEnd = messageQueue.end();
		while (iter != iterEnd) {
			//delete* iter;
			//TODO 동적할당삭제
			iter = messageQueue.erase(iter);
		}
		//messageQueue.erase(remove(messageQueue.begin(), messageQueue.end(),NULL), messageQueue.end());
		ReleaseMutex(hMutex);
	}
public:
	void SendBufferedMessages(Player* player);
};
