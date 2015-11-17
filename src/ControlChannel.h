#ifndef _CONTROL_CHANNEL_H_
#define _CONTROL_CHANNEL_H_

#include <string>
#include "HTTPBody.h"

using namespace std;

class ControlChannel
{
public:
	ControlChannel()
	{
		m_cfg_flag = 0;
		UpdateMac("eth0");
		m_mac_count = 0;
		m_id_count = 0;
		m_start_time = "000000000000";
		m_end_time = "000000000000";
	}
private:
	void UpdateMac(string dev_name);
	void UpdateMemory();
	void UpdateCpu();
	HttpOper *CreateOperHello();
	HttpOper *CreateOperConfig();

	string m_mac;
	string m_mac_no_hyphen;
	string m_memory_info;
	string m_cpu_usage;
	int m_cfg_flag;

	int m_mac_count;
	int m_id_count;
	string m_start_time;
	string m_end_time;
	unsigned char m_ap_mac[6];
};

#endif
