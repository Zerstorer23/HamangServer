#pragma once
#include "Values.h"
typedef struct {
	bool received;
	long lastSentTime;
	long elapsedTime;
} PingRecord, * P_PingRecord;
class IOCP_Server;
class NetworkMessage;
class Player;
class PingManager
{
	unordered_map<int,vector<P_PingRecord>> pingRecords;
	long serverStartTime;
	long serverTimeOffset;
	long serverTime;//keeps onincrementing
public:
	PingManager() {
		serverStartTime = CurrentTimeInMills();
		serverTime = 0;
	}
	~PingManager() {
		for (auto entry : pingRecords) {
			while (!entry.second.empty()) {
				P_PingRecord ping = entry.second.back();
				SAFE_DELETE(ping);
				entry.second.pop_back();
			}
		}
	}


	//호스트 제외 플레이어당 4회씩 핑체크후 평균 / 2

	void RecordPing_Send(int player, long timeSent) {
		P_PingRecord ping = new PingRecord();
		ping->lastSentTime = timeSent;
		ping->received = false; 
		pingRecords[player].push_back(ping);
	}
	int RecordPing_Receive(int player) {
		long timeNow = CurrentTimeInMills();
		P_PingRecord ping = pingRecords[player].back();
		assert(!ping->received);
		ping->elapsedTime = timeNow - ping->lastSentTime;
		cout << "Ping with player " << player << "\t " << ping->elapsedTime << endl;
		ping->received = true;
		return pingRecords[player].size();
	}
	long FinalisePing() {
		long pingSum = 0;
		int pingNum = 0;
		for (auto entry : pingRecords) {
			while (!entry.second.empty()) {
				P_PingRecord ping = entry.second.back();
				pingSum += ping->elapsedTime;
				pingNum++;
				SAFE_DELETE(ping);
				entry.second.pop_back();
			}
		}
		long averagePing = (pingSum) / pingNum;
		serverTimeOffset = averagePing;
		serverStartTime += serverTimeOffset;
		serverTime = CurrentTimeInMills() + serverTimeOffset;
		return serverTime;
	}
	void InitServerTime() {
		serverStartTime = CurrentTimeInMills();
	}

	long CurrentTimeInMills() {
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
	void TestPingStatus(unordered_map<int, Player*> players);
	void PingPlayer(Player* player);
};

