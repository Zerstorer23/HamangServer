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
			if (message.length() + nextMessage.length() < BUFFER - 10) {//¿©À¯
				message.append(nextMessage);
				iter++;
			}
		}
		LPPER_IO_DATA sendIO = serverInstance->CreateMessage(message);
		player->Send(sendIO);
	}
	ReleaseMutex(hMutex);
}

