#ifndef _CONTROL_CHANNEL_H_
#define _CONTROL_CHANNEL_H_

#include <string>
#include <map>
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
	void RunningTableUpdated();

protected:
	ControlChannel();
private:
	void UpdateMac(string dev_name);
	void UpdateMemory();
	void UpdateCpu();
	HttpOper *CreateOperHello();
	HttpOper *CreateOperBaseInfo();
	HttpOper *CreateOperConfig();
	void SendRequest(int first);
	void ProcessResponse(HttpBody *resp);
	void ProcessHelloResponse(HttpOper *oper);
	void ProcessBaseInfoResponse(HttpOper *oper);
	void ProcessDeviceConfigResponse(HttpOper *oper);
	void UpdateRunningParas();
	void UpdateStaticParas();

	std::map<std::string, int> m_configResp;

	string m_mac;
	string m_mac_no_hyphen;
	string m_memory_info;
	string m_cpu_usage;

	/* These are paras from running table */
	int m_runtable_flag;
	std::string m_url;
	int m_invl;
	std::string m_sitename;
	std::string m_siteaddr;
	std::string m_devaddr;
	float m_lon;
	float m_lat;

	/* These are paras from static table */
	std::string m_apid;
	std::string m_devmodel;
	std::string m_detailid;


	int m_cfg_flag;

	int m_mac_count;
	int m_id_count;
	string m_start_time;
	string m_end_time;
	unsigned char m_ap_mac[6];

	int m_should_send;
	int m_should_update;
	int m_send_base_info;

	static void *ThreadLoop(void *arg);
    bool m_exit;
    pthread_t m_tid;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    GeneralTimerItem *m_helloTimer;

	static ControlChannel *m_instance;
};

#endif
