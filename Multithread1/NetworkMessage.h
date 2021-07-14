#pragma once
#include "Values.h"
class NetworkMessage
{
private:
	unsigned int iterator;
public:
	//static wstring ServerSignature;
	string broadcastMessage;
	
	vector<string> tokens;
	vector<int> startPoints;
	int incompleteResumePoint;
	//int lastDelimPoint;

	int count;
	int sentActorNr;
	int targetViewID;
	unsigned int beginPoint;
	unsigned int endPoint;

	NetworkMessage() {
		iterator = 0;
		tokens = {};
		count = 0;
		beginPoint = 0;
		endPoint = 0;
		incompleteResumePoint =0;
		//lastDelimPoint =-1;
	}
	~NetworkMessage() {

	}
	void Split(string& str, char delim) {
		int previous = 0;
		int current = 0;
		tokens.clear();
		current = (int)str.find(delim);
		//find는 찾을게 없으면 npos를 리턴함
		while (current != string::npos) {
			string substring = str.substr(previous, current - previous);
			if (substring.empty()) continue;// __#_#___# <-마지막 캐릭터 empty 스킵
			tokens.push_back(substring);
			startPoints.push_back(previous);
			//cout << substring << endl;
			previous = current + 1;
			current = (int)str.find(delim, previous);
			//if (current != string::npos) {
			//	lastDelimPoint = current;
			//}
		}
//		x.push_back(str.substr(previous, current - previous));
		count = (int)tokens.size();
		//cout << "Token size : "+count << endl;
	//	tokens = x;
	}
	string GetNext() {
		//cout << "Read  : " <<tokens[iterator] << endl;
	//	int i = iterator;
		
		return tokens[iterator++];
	}
	int GetNextInt() {
		return stoi(GetNext());
	}

	void Append(string s)
	{
		broadcastMessage.append(s);
		broadcastMessage.append(NET_DELIM);
		count++;
	}
	void Append(int s)
	{
		Append(to_string(s));
	}

	string SaveStringsForBroadcast() {
		//자신제외 방송용.
		string message;
		for (unsigned int i = beginPoint; i < endPoint; i++) {
			message.append(tokens[i]);
			message.append(NET_DELIM);
			count++;
		}
		broadcastMessage.append(message);
		return message;
	}
	void SetBeginPoint() {
		beginPoint = iterator;
	}
	bool SetEndPoint(int lengthOfMessage) {
		endPoint = beginPoint + lengthOfMessage;
		int beginOfEndBytes = startPoints[beginPoint];
		//cout << "Begin of end bytes " << beginOfEndBytes << endl;
		if (endPoint > tokens.size() ) {
			incompleteResumePoint = beginOfEndBytes;
			//assert(startPoints[beginPoint] > 0);//Need larger buffer; TODO 애초에 버퍼크기가 부족한거임 이건 해결해야하나?
			return false;
		}
		return true;
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
		//서버에서 새로 생성된 메세지용
		return NET_SIG+NET_DELIM+ to_string(count + 2).append(NET_DELIM).append(broadcastMessage);
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
		cout << endl;

	}

	static string MsgInfoToString(MessageInfo info) {
		switch (info)
		{
		case MessageInfo::ServerRequest:
			return u8"Server Request";
		case MessageInfo::RPC:
			return u8"RPC";
		case MessageInfo::SyncVar:
			return u8"SyncVar";
		case MessageInfo::Chat:
			return u8"Chat";
		case MessageInfo::Instantiate:
			return u8"Instantiate";
		case MessageInfo::Destroy:
			return u8"Destroy";
		case MessageInfo::SetHash:
			return u8"SetHash";
		case MessageInfo::ServerCallbacks:
			return u8"ServerCallbacks";
		default:
			return u8"?";
		}

	}
	static string ReqInfoToString(LexRequest info) {
		switch (info)
		{
		case LexRequest::RemoveRPC:
			return u8"RemoveRPC";
		case LexRequest::ChangeMasterClient:
			return u8"ChangeMasterClient";
		case LexRequest::Receive_modifiedTime:
			return u8"Receive_modifiedTime";
		case LexRequest::Ping:
			return u8"Ping";
		default:
			return u8"?";
		}
	}

	static void convert_utf8_to_unicode_string(
		__out wstring& unicode,
		__in const char* utf8,
		__in const size_t utf8_size
	) {
		unicode.clear();

		int required_cch = ::MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			utf8, static_cast<int>(utf8_size),
			nullptr, 0
		);
		unicode.resize(required_cch);
		MultiByteToWideChar(
			CP_UTF8,
			MB_ERR_INVALID_CHARS,
			utf8, static_cast<int>(utf8_size),
			const_cast<wchar_t*>(unicode.c_str()), static_cast<int>(unicode.size())
		);
	}
	static void convert_utf8_to_unicode_string2(
		char* utf8,
		wchar_t* unicode,
		const size_t utf8_size
	) {
		int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8, strlen(utf8), NULL, NULL);
		MultiByteToWideChar(CP_UTF8, 0, utf8, strlen(utf8), unicode, nLen);
	}

	//
// convert_unicode_to_utf8_string
//
	static void convert_unicode_to_utf8_string(
		__out string& utf8,
		__in const wchar_t* unicode,
		__in const size_t unicode_size
	) {

		utf8.clear();
		int required_cch = ::WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			unicode, static_cast<int>(unicode_size),
			nullptr, 0,
			nullptr, nullptr
		);
		utf8.resize(required_cch);
		WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			unicode, static_cast<int>(unicode_size),
			const_cast<char*>(utf8.c_str()), static_cast<int>(utf8.size()),
			nullptr, nullptr
		);

	}
};

