#include "Player.h"
#include "NetworkMessage.h"
#include "IOCP_Server.h"
#include "HashTable.h"
Player::Player()
{
	handleInfo = nullptr;
	actorNumber = -1;
	unique_id = "";
	isMasterClient = false;
	shared_ptr<HashTable>  cp(new HashTable());
	customProperty = cp;
}
void Player::SetActorNumber(int id)
{
	actorNumber = id;
	customProperty->name = " Client " + id;
}
void Player::Send(char* sendBuffer, DWORD& bytesReceived)
{
	LPPER_IO_DATA cloneIO = IOCP_Server::GetInst()->CloneBufferData(sendBuffer, bytesReceived, WRITE);
	SOCKET targetSocket = handleInfo->clientSocket;
	cout << sendBuffer << "Send to Actor number " << actorNumber << endl;
	WSASend(targetSocket, &(cloneIO->wsaBuf), 1, NULL, 0, &(cloneIO->overlapped), NULL);
}

void Player::Send(LPPER_IO_DATA sendIO)
{
	SOCKET targetSocket = handleInfo->clientSocket;
	cout << "Send to Actor number " << actorNumber << endl;
	WSASend(targetSocket, &(sendIO->wsaBuf), 1, NULL, 0, &(sendIO->overlapped), NULL);
}

void Player::EncodeToNetwork(NetworkMessage& netMessage)
{
	netMessage.Append(to_string(actorNumber));
	netMessage.Append(to_string(isMasterClient));
	customProperty->EncodeToNetwork(netMessage);
}
