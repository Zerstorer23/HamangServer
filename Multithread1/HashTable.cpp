
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
	name = "Server";
	propertyMutex = CreateMutex(NULL, FALSE, NULL);
}

HashTable::~HashTable()
{
	customProperty.clear();
}
