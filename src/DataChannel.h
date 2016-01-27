#ifndef _DATA_CHANNEL_H_
#define _DATA_CHANNEL_H_

#include <string>
#include <pthread.h>
#include "GeneralTimer.h"

using namespace std;

#define DATA_INSTANCE DataChannel::Instance()

#define TX_TIMER_NAME      "txtimer"

class DataChannel
{
public:
	~DataChannel();
	static DataChannel *Instance();
	static void Destroy();
	void Start();
	void Stop();
	void RunningTableUpdated();
	void HandleTimer(std::string &name, void *data);
	int GetStartTime();
	int GetEndTime();
	int GetSendNum();
	void FirstSendData();

protected:
	DataChannel();

private:
	std::string m_addr;
	int m_port;
	int m_invl;

	static void *ThreadLoop(void *arg);
	void UpdateRunningParas();
	void UpdateTxTimer(int invl);
	int Connect();
	void Close();
	int Send();

    bool m_exit;
    pthread_t m_tid;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;

    int m_should_update;
    int m_should_send;
    
    int m_start_time;
    int m_end_time;
    int m_number;

    int m_socket;

    GeneralTimerItem *m_txTimer;

	static DataChannel *m_instance;
};

#endif
