#pragma once
#include "Values.h"
typedef struct {
	bool received;
	long long  lastSentTime;
	long long  elapsedTime;
} PingRecord, * P_PingRecord;
class IOCP_Server;
class NetworkMessage;
class Player;
class PlayerManager;
class PingManager
{
	unordered_map<int,P_PingRecord> pingRecords;
	static long long  serverTime;//keeps onincrementing
public:
	PingManager();
	~PingManager() {

	}
	void UpdateTime()
	{
		serverTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
	//호스트 제외 플레이어당 4회씩 핑체크후 평균 / 2

	void RecordPing_Send(int player, long long timeSent) {
		P_PingRecord ping = new PingRecord();
		ping->lastSentTime = timeSent;
		ping->received = false; 
		pingRecords[player]=ping;
	}
	void RecordPing_Receive(Player* player);

	long long  CurrentTimeInMills() {
		return  duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
	void TimeCheck() {
		for (int i = 0; i < 10; i++) {
			auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
			auto sec_since_epoch = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

			cout << "seconds since epoch: " << sec_since_epoch << endl;
			cout << "milliseconds since epoch: " << millisec_since_epoch << endl;

			this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return ;
	}
	void Handle_Request_TimeSynch(NetworkMessage& netMessage);
	void PushServerTimeToPlayer(Player* player, int isModification, long long timeValue);
};

