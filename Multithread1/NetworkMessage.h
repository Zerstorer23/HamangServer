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
		//find�� ã���� ������ npos�� ������
		while (current != string::npos) {
			string substring = str.substr(previous, current - previous);
			if (substring.empty()) continue;// __#_#___# <-������ ĳ���� empty ��ŵ
			x.push_back(substring);
			cout << substring << endl;
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
		//��۸޼���
		return broadcastMessage;//�߿�. ó���� net delim���� �ȵ����� c#���� ���鼭 ���������
	}
	string BuildNewSignedMessage() {
		//�߿�. ó���� net delim���� �ȵ����� c#���� ���鼭 ���������
		//�������� ���� ������ �޼�����
		return "#LEX#"+ to_string(count+2)+ broadcastMessage;
	}
};

