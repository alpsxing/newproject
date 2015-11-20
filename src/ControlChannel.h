#ifndef _CONTROL_CHANNEL_H_
#define _CONTROL_CHANNEL_H_

#include <string>
#include <pthread.h>
#include "GeneralTimer.h"
#include "HTTPBody.h"

using namespace std;

#define CONTROL_INSTANCE ControlChannel::Instance()

#define HELLO_TIMER_NAME      "hellotimer"

class ControlChannel
{
public:
	~ControlChannel();
	static ControlChannel *Instance();
	static void Destroy();
	void Start();
	void Stop();
	void HandleTimer(string &name, void *data);
	void ChangeHelloInvl(int invl, int start);

protected:
	ControlChannel();
private:
	void UpdateMac(string dev_name);
	void UpdateMemory();
	void UpdateCpu();
	HttpOper *CreateOperHello();
	HttpOper *CreateOperConfig();
	void SendRequest(int first);
	void ProcessResponse(HttpBody *resp);
	void ProcessHelloResponse(HttpOper *oper);
	void ProcessConfigResponse(HttpOper *oper);

	string m_mac;
	string m_mac_no_hyphen;
	string m_memory_info;
	string m_cpu_usage;
	int m_cfg_flag;
	int m_runtable_flag;
	string m_url;

	int m_mac_count;
	int m_id_count;
	string m_start_time;
	string m_end_time;
	unsigned char m_ap_mac[6];

	int m_should_send;

	static void *ThreadLoop(void *arg);
    bool m_exit;
    pthread_t m_tid;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    GeneralTimerItem *m_helloTimer;

	static ControlChannel *m_instance;
};

#endif
