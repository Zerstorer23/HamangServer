
#include "HashTable.h"
#include "NetworkMessage.h"
void HashTable::EncodeToNetwork(NetworkMessage& netMessage)
{
	netMessage.Append(to_string(customProperty.size()));
	for (auto entry : customProperty) {
		netMessage.Append(entry.first);
		netMessage.Append(entry.second->typeName);
		netMessage.Append(entry.second->value);
	}
}

HashTable::HashTable()
{
	name = u8"Server"s;
	propertyMutex = CreateMutex(NULL, FALSE, NULL);
}

HashTable::~HashTable()
{
	customProperty.clear();
}

void HashTable::SetProperty(string key, string typeName, string value)
{
	shared_ptr<HashValue>  pairKey(new HashValue());
	pairKey->typeName = typeName;
	pairKey->value = value;
	//cout << value << endl;
	//wstring comp = L"6월26일";
	////utf-8 -> wstring 유니코드[저장]
	////보낼떄는 wstring unicode -> utf-8
	//const char* client = value.c_str();
	//string utf8str;
	//DWORD res = NetworkMessage::convert_unicode_to_utf8_string(utf8str, comp.c_str(), comp.size());
	//cout << "RES " << res << endl;
	//const char* server = utf8str.c_str();

	//for (int i = 0; i < value.size(); i++) {
	//	cout << (int)client[i] << " vs " << (int)server[i] << endl;
	//}
	WaitForSingleObject(propertyMutex, INFINITE);
	customProperty.insert_or_assign(key, pairKey);
	ReleaseMutex(propertyMutex);
}
