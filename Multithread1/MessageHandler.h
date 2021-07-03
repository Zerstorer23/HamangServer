#pragma once
#include "Values.h"
class Player;
class NetworkMessage;
class MessageHandler
{
public:
	MessageHandler() {
	};
	~MessageHandler() {
	};

	static void HandleMessage(NetworkMessage& netMessage);
	static void Handle_PropertyRequest(NetworkMessage& netMessage);
	static void Handle_ServerRequest(NetworkMessage& netMessage);
	//static void Handle_BroadcastString(NetworkMessage& netMessage);
	//static void Handle_ServerRequest_SendBufferedRPCs(Player* target);
	static void Handle_ServerRequest_RemoveRPCs(NetworkMessage& netMessage);
	static void Handle_ServerRequest_ReceiveModifiedTime(NetworkMessage& netMessage);
	static void Handle_ServerRequest_ChangeMasterClient(NetworkMessage& netMessage);
	static void Handle_ServerRequest_Ping(NetworkMessage& netMessage);
};

