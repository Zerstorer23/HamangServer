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

	BufferedMessages() {

		hMutex = CreateMutex(NULL, FALSE, NULL);
	}
	~BufferedMessages() {
		for (auto message : messageQueue) {
			SAFE_DELETE(message);
		}
	}

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
				messageQueue.erase(iter);
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
				messageQueue.erase(iter);
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
				messageQueue.erase(iter);
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
			SAFE_DELETE(*iter);
			messageQueue.erase(iter);
		}
		ReleaseMutex(hMutex);
	}
	void SendBufferedMessages(Player* player);
};
