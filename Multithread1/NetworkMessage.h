#pragma once
#include "Values.h"
class NetworkMessage
{
private:
	unsigned int iterator;
public:
	string broadcastMessage;
	vector<string> tokens;
	int count;
	int sentActorNr;
	int targetViewID;
	int beginPoint;
	int endPoint;

	NetworkMessage() {
		iterator = 0;
		tokens = {};
		count = 0;
		beginPoint = 0;
		endPoint = 0;
	}
	~NetworkMessage() {
	
	}
	void Split(string& str, char delim) {
		int previous = 0;
		int current = 0;
		vector<string> x;
		x.clear();
		current = (int)str.find(delim);
		//find는 찾을게 없으면 npos를 리턴함
		while (current != string::npos) {
			string substring = str.substr(previous, current - previous);
			if (substring.empty()) continue;// __#_#___# <-마지막 캐릭터 empty 스킵
			x.push_back(substring);
			//cout << substring << endl;
			previous = current + 1;
			current = (int)str.find(delim, previous);
		}
		x.push_back(str.substr(previous, current - previous));
		count =(int) x.size();
		tokens =  x;
	}
	string GetNext() {
		return tokens[iterator++];
	}
	string PeekPrev() {
		return tokens[iterator - 1];
	}
	
	void Append(string s)
	{
		broadcastMessage.append(NET_DELIM);
		broadcastMessage.append(s);
		count++;
	}
	void Append(int s)
	{
		broadcastMessage.append(NET_DELIM);
		broadcastMessage.append(to_string(s));
		count++;
	}

	string SaveStrings() {
		//자신제외 방송용.
		string message;
		for (int i = beginPoint; i < endPoint; i++) {
			message.append(NET_DELIM);
			message.append(tokens[i]);
			count++;
		}
		broadcastMessage.append(message);
		cout << "Saved string: " << broadcastMessage << endl;
		return message;
	}
	void SetBeginPoint() {
		beginPoint = iterator;
	}
	void SetEndPoint(int lengthOfMessage) {
		endPoint = beginPoint + lengthOfMessage;
	}
	void SetIteratorToEnd() {
		iterator = endPoint;
	}

	bool HasNext() {
		return iterator < tokens.size();
	}

	bool HasMessageToBroadcast() {
		return !broadcastMessage.empty();
	}

	string BuildCopiedMessage() {
		//방송메세지
		return broadcastMessage;//중요. 처음에 net delim들어가면 안되지만 c#으로 가면서 사라져버림
	}
	string BuildNewSignedMessage() {
		//중요. 처음에 net delim들어가면 안되지만 c#으로 가면서 사라져버림
		//서버에서 새로 생성된 메세지용
		return "#LEX#"+ to_string(count+2)+ broadcastMessage;
	}
	void PrintOut() {
		int iter = beginPoint;
		int initial = beginPoint;
		iter++;
		int length = stoi(tokens[iter++]);
		string sentActor = tokens[iter++];
		MessageInfo msgInfo = (MessageInfo)stoi(tokens[iter++]);
		if (msgInfo == MessageInfo::ServerRequest) {
			LexRequest reqInfo = (LexRequest)stoi(tokens[iter++]);
			if (reqInfo == LexRequest::Ping) return;
			cout << "===============================" << endl;
			cout << sentActor << " : " << MsgInfoToString(msgInfo);
			cout << " - " << ReqInfoToString(reqInfo);
		}
		while (iter < initial + length) {
			cout << " " << tokens[iter++];
		}
		cout <<endl;

	}

	static string MsgInfoToString(MessageInfo info) {
		switch (info)
		{
		case MessageInfo::ServerRequest:
			return "Server Request";
		case MessageInfo::RPC:
			return "RPC";
		case MessageInfo::SyncVar:
			return "SyncVar";
		case MessageInfo::Chat:
			return "Chat";
		case MessageInfo::Instantiate:
			return "Instantiate";
		case MessageInfo::Destroy:
			return "Destroy";
		case MessageInfo::SetHash:
			return "SetHash";
		case MessageInfo::ServerCallbacks:
			return "ServerCallbacks";
		default:
			return "?";
		}
	
	}
	static string ReqInfoToString(LexRequest info) {
		switch (info)
		{
		case LexRequest::RemoveRPC:
			return "RemoveRPC";
		case LexRequest::ChangeMasterClient:
			return "ChangeMasterClient";
		case LexRequest::Receive_modifiedTime:
			return "Receive_modifiedTime";
		case LexRequest::Ping:
			return "Ping";
		default:
			return "?";
		}
	}
};

