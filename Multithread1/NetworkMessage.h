#pragma once
#include "Values.h"
class NetworkMessage
{
private:
	unsigned int iterator;
public:
	wstring broadcastMessage;
	vector<wstring> tokens;
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
	void Split(wstring& str, wchar_t delim) {
		int previous = 0;
		int current = 0;
		vector<wstring> x;
		x.clear();
		current = (int)str.find(delim);
		//find는 찾을게 없으면 npos를 리턴함
		while (current != wstring::npos) {
			wstring substring = str.substr(previous, current - previous);
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
	wstring GetNext() {
		return tokens[iterator++];
	}
	wstring PeekPrev() {
		return tokens[iterator - 1];
	}
	
	void Append(wstring s)
	{
		broadcastMessage.append(NET_DELIM);
		broadcastMessage.append(s);
		count++;
	}
	void Append(int s)
	{
		broadcastMessage.append(NET_DELIM);
		broadcastMessage.append(to_wstring(s));
		count++;
	}

	wstring SaveStrings() {
		//자신제외 방송용.
		wstring message;
		for (int i = beginPoint; i < endPoint; i++) {
			message.append(NET_DELIM);
			message.append(tokens[i]);
			count++;
		}
		broadcastMessage.append(message);
		wcout << L"Saved string: " << broadcastMessage << endl;
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

	wstring BuildCopiedMessage() {
		//방송메세지
		return broadcastMessage;//중요. 처음에 net delim들어가면 안되지만 c#으로 가면서 사라져버림
	}
	wstring BuildNewSignedMessage() {
		//중요. 처음에 net delim들어가면 안되지만 c#으로 가면서 사라져버림
		//서버에서 새로 생성된 메세지용
		return L"#LEX#"+ to_wstring(count+2)+ broadcastMessage;
	}
	void PrintOut() {
		int iter = beginPoint;
		int initial = beginPoint;
		iter++;
		int length = stoi(tokens[iter++]);
		wstring sentActor = tokens[iter++];
		MessageInfo msgInfo = (MessageInfo)stoi(tokens[iter++]);
		if (msgInfo == MessageInfo::ServerRequest) {
			LexRequest reqInfo = (LexRequest)stoi(tokens[iter++]);
			if (reqInfo == LexRequest::Ping) return;
			wcout << "===============================" << endl;
			wcout << sentActor << " : " << MsgInfoToString(msgInfo);
			wcout << " - " << ReqInfoToString(reqInfo);
		}
		while (iter < initial + length) {
			wcout << " " << tokens[iter++];
		}
		wcout <<endl;

	}

	static wstring MsgInfoToString(MessageInfo info) {
		switch (info)
		{
		case MessageInfo::ServerRequest:
			return L"Server Request";
		case MessageInfo::RPC:
			return L"RPC";
		case MessageInfo::SyncVar:
			return L"SyncVar";
		case MessageInfo::Chat:
			return L"Chat";
		case MessageInfo::Instantiate:
			return L"Instantiate";
		case MessageInfo::Destroy:
			return L"Destroy";
		case MessageInfo::SetHash:
			return L"SetHash";
		case MessageInfo::ServerCallbacks:
			return L"ServerCallbacks";
		default:
			return L"?";
		}
	
	}
	static wstring ReqInfoToString(LexRequest info) {
		switch (info)
		{
		case LexRequest::RemoveRPC:
			return L"RemoveRPC";
		case LexRequest::ChangeMasterClient:
			return L"ChangeMasterClient";
		case LexRequest::Receive_modifiedTime:
			return L"Receive_modifiedTime";
		case LexRequest::Ping:
			return L"Ping";
		default:
			return L"?";
		}
	}

	static DWORD convert_utf8_to_unicode_string(
		__out wstring& unicode,
		__in const char* utf8,
		__in const size_t utf8_size
	) {
		DWORD error = 0;
		do {
			if ((nullptr == utf8) || (0 == utf8_size)) {
				error = ERROR_INVALID_PARAMETER;
				break;
			}
			unicode.clear();
			//
			// getting required cch.
			//
			int required_cch = ::MultiByteToWideChar(
				CP_UTF8,
				MB_ERR_INVALID_CHARS,
				utf8, static_cast<int>(utf8_size),
				nullptr, 0
			);
			if (0 == required_cch) {
				error = ::GetLastError();
				break;
			}
			//
			// allocate.
			//
			unicode.resize(required_cch);
			//
			// convert.
			//
			if (0 == ::MultiByteToWideChar(
				CP_UTF8,
				MB_ERR_INVALID_CHARS,
				utf8, static_cast<int>(utf8_size),
				const_cast<wchar_t*>(unicode.c_str()), static_cast<int>(unicode.size())
			)) {
				error = ::GetLastError();
				break;
			}
		} while (false);
		return error;
	}

	//
// convert_unicode_to_utf8_string
//
	static DWORD convert_unicode_to_utf8_string(
		__out string& utf8,
		__in const wchar_t* unicode,
		__in const size_t unicode_size
	) {
		DWORD error = 0;
		do {
			if ((nullptr == unicode) || (0 == unicode_size)) {
				error = ERROR_INVALID_PARAMETER;
				break;
			}
			utf8.clear();
			//
			// getting required cch.
			//
			int required_cch = ::WideCharToMultiByte(
				CP_UTF8,
				WC_ERR_INVALID_CHARS,
				unicode, static_cast<int>(unicode_size),
				nullptr, 0,
				nullptr, nullptr
			);
			if (0 == required_cch) {
				error = ::GetLastError();
				break;
			}
			//
			// allocate.
			//
			utf8.resize(required_cch);
			//
			// convert.
			//
			if (0 == ::WideCharToMultiByte(
				CP_UTF8,
				WC_ERR_INVALID_CHARS,
				unicode, static_cast<int>(unicode_size),
				const_cast<char*>(utf8.c_str()), static_cast<int>(utf8.size()),
				nullptr, nullptr
			)) {
				error = ::GetLastError();
				break;
			}
		} while (false);
		return error;
	}
};

