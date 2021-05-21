#include "Player.h"
#include "NetworkMessage.h"
#include "IOCP_Server.h"
void Player::Send(char* sendBuffer, DWORD& bytesReceived)
{
	LPPER_IO_DATA cloneIO = IOCP_Server::GetInst()->CloneBufferData(sendBuffer, bytesReceived, WRITE);
	SOCKET targetSocket = handleInfo->clientSocket;
	cout << sendBuffer << "Send to Actor number " << actorNumber << endl;
	WSASend(targetSocket, &(cloneIO->wsaBuf), 1, NULL, 0, &(cloneIO->overlapped), NULL);
}

void Player::EncodeToNetwork(NetworkMessage& netMessage)
{
	netMessage.Append(to_string(actorNumber));
	netMessage.Append(to_string(isMasterClient));
	netMessage.Append(to_string(CustomProperty.size()));
	for (auto entry : CustomProperty) {
		netMessage.Append(entry.first);
		netMessage.Append(entry.second);
	}
}
