#pragma once

#include "Values.h"
typedef struct {
	wstring typeName;
	wstring value;
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
	wstring name;
	HANDLE propertyMutex;
	unordered_map<wstring, shared_ptr<HashValue>> customProperty;
	HashTable(); 
	~HashTable();
	void SetProperty(wstring key, wstring typeName, wstring value);
	void RemoveAllProperties() {
		WaitForSingleObject(propertyMutex, INFINITE);
		customProperty.clear();
		ReleaseMutex(propertyMutex);
	}
	wstring EncodeToNetwork() {
		wstring message = NET_DELIM;
		message = message.append(to_wstring(customProperty.size()));
		for (auto entry : customProperty) {
			auto value = entry.second;
			message = message.append(NET_DELIM).append(entry.first).append(NET_DELIM).append(value->typeName).append(NET_DELIM).append(value->value);
		}
		wcout << "Room: " << message << endl;
		return message;
	}
	void EncodeToNetwork(NetworkMessage& netMessage);
	void PrintProperties() {
		cout << endl;
		for (auto entry : customProperty) {
			wcout << name<<" |\t" << entry.first << "|\t" << entry.second->typeName << "|\t" << entry.second->value << endl;
		}
	}


};

