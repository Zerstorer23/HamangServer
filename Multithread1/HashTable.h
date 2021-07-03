#pragma once

#include "Values.h"
typedef struct {
	string typeName;
	string value;
}HashValue;
//"aa가"
/*
  길이 확인하고
  길이가 다르면 읽는방법을 통일 <- 

*/
class NetworkMessage;
class HashTable
{
public:
	string name;
	HANDLE propertyMutex;
	unordered_map<string, shared_ptr<HashValue>> customProperty;
	HashTable(); 
	~HashTable();
	void SetProperty(string key, string typeName, string value);
	void RemoveAllProperties() {
		WaitForSingleObject(propertyMutex, INFINITE);
		customProperty.clear();
		ReleaseMutex(propertyMutex);
	}
	string EncodeToNetwork() {
		string message = NET_DELIM;
		message = message.append(to_string(customProperty.size()));
		for (auto entry : customProperty) {
			auto value = entry.second;
			message = message.append(NET_DELIM).append(entry.first).append(NET_DELIM).append(value->typeName).append(NET_DELIM).append(value->value);
		}
		cout << "Room: " << message << endl;
		return message;
	}
	void EncodeToNetwork(NetworkMessage& netMessage);
	void PrintProperties() {
		cout << endl;
		for (auto entry : customProperty) {
			cout << name<<" |\t" << entry.first << "|\t" << entry.second->typeName << "|\t" << entry.second->value << endl;
		}
	}


};

