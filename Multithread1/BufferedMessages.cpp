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
		message.reset();
	}
}
void BufferedMessages::EnqueueMessage(int playerNr, int viewID, wstring message) {
	WaitForSingleObject(hMutex, INFINITE);
	shared_ptr<RPC>  rpc(new RPC());
	rpc->playerActorNr = playerNr;
	rpc->viewID = viewID;
	rpc->message = message; //����Ʈ������ Ȱ�� TODO
	//auto a = rpc; // ī��Ʈ2

	messageQueue.push_back(rpc);
//unique�� user count 1 �̻� �Ұ�..

	ReleaseMutex(hMutex);
}
void BufferedMessages::RemovePlayerNr(int playerNr) {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "playerNr �������� " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->playerActorNr == playerNr) {
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	cout << "RPCũ�� " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}//������	
void BufferedMessages::RemoveViewID(int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "viewID �������� " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->viewID == viewID) {
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	cout << "RPCũ�� " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}
void BufferedMessages::RemoveRPC(int playerNr, int viewID) {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "player viewID �������� " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		if ((*iter)->viewID == viewID && (*iter)->playerActorNr == playerNr) {
			iter->reset();
			iter = messageQueue.erase(iter);
		}
		else {
			iter++;
		}
	}
	cout << "RPCũ�� " << messageQueue.size() << endl;
	ReleaseMutex(hMutex);

}
void BufferedMessages::RemoveAll() {
	WaitForSingleObject(hMutex, INFINITE);
	cout << "��ü���� ���� " << messageQueue.size() << endl;
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	while (iter != iterEnd) {
		//cout << iter->use_count() << endl;
		iter->reset();
		iter = messageQueue.erase(iter);
	}//for delete ��������clear
	//messageQueue.erase(remove(messageQueue.begin(), messageQueue.end(),NULL), messageQueue.end());
	cout <<"RPCũ�� "<< messageQueue.size() << endl;
	ReleaseMutex(hMutex);
}
void BufferedMessages::SendBufferedMessages(Player* player)
{
	WaitForSingleObject(hMutex, INFINITE);
	auto iter = messageQueue.begin();
	auto iterEnd = messageQueue.end();
	IOCP_Server* serverInstance = IOCP_Server::GetInst();
	while (iter != iterEnd) {
		wstring message = (*iter)->message;
		iter++;
		while (iter != iterEnd) {
			wstring nextMessage = (*iter)->message;
			if (message.length() + nextMessage.length() < BUFFER - 10) {//����
				message.append(nextMessage);
				iter++;
			}
		}
		wcout << "Buffered RPC " << message << endl;
		player->Send(message);
	}
	//TODO Buffered rpc �Ⱥ�����
	ReleaseMutex(hMutex);
}
