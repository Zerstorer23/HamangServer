#pragma once
#include "Values.h"
typedef struct {
	//bool received;
	long long lastSentTime;
	//long long elapsedTime;
} PingRecord, * P_PingRecord;
class IOCP_Server;
class NetworkMessage;
class Player;
class PlayerManager;
class PingManager
{
private:
	unordered_map<int,long long> pingRecords;
	long long  serverTime;//keeps onincrementing

	static PingManager* instance;
public:
	static PingManager* GetInst() {
		if (!instance) {
			instance = new PingManager;
			return instance;
		}
		else {
			return instance;
		}
	}
	static void DestroyInst() {
		SAFE_DELETE(instance);
	}
private:
	PingManager();
	~PingManager();

public:
	void UpdateTime()
	{
		serverTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
	long long GetTimeNow() {
		UpdateTime();
		return serverTime;
	}
	//호스트 제외 플레이어당 4회씩 핑체크후 평균 / 2

	void RecordPing(int player, long long timeSent) {
	//	P_PingRecord ping = new PingRecord();
		//ping->lastSentTime = timeSent;
		//ping->received = false; 
		pingRecords[player]= timeSent;
		cout << "Init Time" << timeSent << endl;
	}
	long long  CurrentTimeInMills() {
		return  duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}
	void TimeCheck() {
		for (int i = 0; i < 10; i++) {
			cout << "ServerTime: " << GetTimeNow() << endl;
			this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		return ;
	}
	void TimeSynch_Receive(Player* player);

	void TimeSync_Send(Player* player, long long timeValue);
};

