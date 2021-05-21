#pragma once
#include "Values.h"
class PingManager
{


	unordered_map<int,vector<long>> * pingRecords;
	long serverStartTime;
	long serverTime;
public:
	PingManager() {
		pingRecords = nullptr;
		serverStartTime = 0;
		serverTime = 0;
	}
	~PingManager() {
		SAFE_DELETE(pingRecords)
	}
	//호스트 제외 플레이어당 4회씩 핑체크후 평균 / 2

	bool RecordPing(int player, long elapsed) {
	
		return false;
	}
	long FinalisePing() {

		return serverTime;
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
};

