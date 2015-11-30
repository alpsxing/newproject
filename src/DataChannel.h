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

protected:
	DataChannel();

private:
	std::string m_addr;
	int m_port;
	int m_invl;

	static void *ThreadLoop(void *arg);
	void UpdateRunningParas();
	void UpdateTxTimer();
    bool m_exit;
    pthread_t m_tid;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;

    int m_should_update;
    int m_should_send;

    GeneralTimerItem *m_txTimer;

	static DataChannel *m_instance;
};

#endif
