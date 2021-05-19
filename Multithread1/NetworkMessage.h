#pragma once
#include "Values.h"
class NetworkMessage
{
public:
	string broadcastMessage;
	unsigned int iterator;
	vector<string> tokens;
	int count;
	
	NetworkMessage() {
		iterator = 0;
		tokens = {};
		count = 0;
	}
	~NetworkMessage() {
	
	}
	void Split(string& str, char delim) {
		int previous = 0;
		int current = 0;
		vector<string> x;
		x.clear();
		current = str.find(delim);
		//find�� ã���� ������ npos�� ������
		while (current != string::npos) {
			string substring = str.substr(previous, current - previous);
			if (substring.empty()) continue;// __#_#___# <-������ ĳ���� empty ��ŵ
			x.push_back(substring);
			cout << substring << endl;
			previous = current + 1;
			current = str.find(delim, previous);
		}
		x.push_back(str.substr(previous, current - previous));
		count = x.size();
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
	//void AppendPrevious()
	//{
	//	broadcastMessage.append(NET_DELIM);
	//	broadcastMessage.append(tokens[iterator-1]);
	//}
	void SaveStrings(int begin, int end) {
		for (int i = begin; i < end; i++) {
			broadcastMessage.append(NET_DELIM);
			broadcastMessage.append(tokens[i]);
			count++;
		}
		cout << "Saved string: " << broadcastMessage << endl;
	}
	bool HasNext() {
		return iterator < tokens.size();
	}

	bool HasMessageToBroadcast() {
		return !broadcastMessage.empty();
	}
	string BuildCopiedMessage() {
		return broadcastMessage;//�߿�. ó���� net delim���� �ȵ����� c#���� ���鼭 ���������
	}
	string BuildNewSignedMessage() {
		return "#LEX#"+ to_string(count+2)+ broadcastMessage;//�߿�. ó���� net delim���� �ȵ����� c#���� ���鼭 ���������
	}
};

