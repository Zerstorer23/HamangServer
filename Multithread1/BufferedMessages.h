#pragma once
#include "Values.h"

class Player;
class IOCP_Server;
class NeworkMessage;
class BufferedMessages
{
private:
	list<PRPC> messageQueue;
public:
	HANDLE hMutex;
	DECLARE_SINGLE(BufferedMessages)

public:
	void EnqueueMessage(int playerNr, int viewID, string message);
	
	void RemovePlayerNr(int playerNr);

	void RemoveViewID(int viewID) {
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
	void RemoveRPC(int playerNr, int viewID) {
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
	void RemoveAll() {
		WaitForSingleObject(hMutex, INFINITE);
		auto iter = messageQueue.begin();
		auto iterEnd = messageQueue.end();
		while (iter != iterEnd) {
			delete* iter;
			iter = messageQueue.erase(iter);
		}//for delete 마지막에clear
		//messageQueue.erase(remove(messageQueue.begin(), messageQueue.end(),NULL), messageQueue.end());
		ReleaseMutex(hMutex);
	}
public:
	void SendBufferedMessages(Player* player);
};
