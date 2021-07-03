#include "Player.h"
#include "NetworkMessage.h"
#include "IOCP_Server.h"
#include "HashTable.h"
Player::Player()
{
	handleInfo = nullptr;
	actorNumber = -1;
	unique_id = u8""s;
	isMasterClient = false;
	isConnected = false;
	shared_ptr<HashTable>  cp(new HashTable());
	customProperty = cp;
}
void Player::SetActorNumber(int id)
{
	actorNumber = id;
	customProperty->name = u8"Client "s + to_string(id);
}
void Player::Send(string message, bool isInitial)
{
	if (!isConnected && !isInitial) return;
	//string u8message;
	//NetworkMessage::convert_unicode_to_utf8_string(u8message, message.c_str(), message.length());
	LPPER_IO_DATA cloneIO = IOCP_Server::GetInst()->CreateMessageBuffer(message, WRITE);
	SOCKET targetSocket = handleInfo->clientSocket;
	DEBUG_MODE cout << "["<< message << "] to Player " << actorNumber << endl;
	WSASend(targetSocket, &(cloneIO->wsaBuf), 1, NULL, 0, &(cloneIO->overlapped), NULL);
}

void Player::EncodeToNetwork(NetworkMessage& netMessage)
{
	netMessage.Append(to_string(actorNumber));
	netMessage.Append(to_string(isMasterClient));
	customProperty->EncodeToNetwork(netMessage);
}
