#include "Player.h"
#include "NetworkMessage.h"
#include "IOCP_Server.h"
#include "HashTable.h"
Player::Player()
{
	handleInfo = nullptr;
	actorNumber = -1;
	unique_id = L"";
	isMasterClient = false;
	shared_ptr<HashTable>  cp(new HashTable());
	customProperty = cp;
}
void Player::SetActorNumber(int id)
{
	actorNumber = id;
	customProperty->name = L"Client " + id;
}
void Player::Send(wstring message)
{
	string u8message;
	NetworkMessage::convert_unicode_to_utf8_string(u8message, message.c_str(), message.length());
	LPPER_IO_DATA cloneIO = IOCP_Server::GetInst()->CreateMessageBuffer(u8message, WRITE);
	SOCKET targetSocket = handleInfo->clientSocket;
	DEBUG_MODE cout << "["<<u8message << "] to Player " << actorNumber << endl;
	WSASend(targetSocket, &(cloneIO->wsaBuf), 1, NULL, 0, &(cloneIO->overlapped), NULL);
}

void Player::EncodeToNetwork(NetworkMessage& netMessage)
{
	netMessage.Append(to_wstring(actorNumber));
	netMessage.Append(to_wstring(isMasterClient));
	customProperty->EncodeToNetwork(netMessage);
}
