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

	void RemoveViewID(int viewID);
	void RemoveRPC(int playerNr, int viewID);
	void RemoveAll();
	void SendBufferedMessages(Player* player);
};

