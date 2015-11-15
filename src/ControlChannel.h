#ifndef _CONTROL_CHANNEL_H_
#define _CONTROL_CHANNEL_H_

#include <string>

using namespace std;

class ControlChannel
{
public:
	ControlChannel()
	{
		m_cfg_flag = 0;
		UpdateMac("eth0");
	}
private:
	void UpdateMac(string dev_name);
	void UpdateMemory();
	void UpdateCpu();

	string m_mac;
	string m_memory_info;
	string m_cpu_usage;
	int m_cfg_flag;
};

#endif
