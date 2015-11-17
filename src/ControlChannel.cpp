#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include "boost/format.hpp"
#include "ControlChannel.h"
#include "boost/algorithm/string.hpp"
#include "Utility.h"
#include "version.h"
#include "RunningConfigTable.h"
#include "StaticConfigTable.h"
using boost::format;

typedef vector<string> split_vector_type;
using namespace boost;

void ControlChannel::UpdateMac(string dev_name)
{
	struct ifreq ifr;
	int sock;
	const unsigned char* mac;

	memset(m_ap_mac, 0, sizeof(m_ap_mac));

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock = -1)
		goto MAKESTRING;
	strncpy(ifr.ifr_name, dev_name.c_str(), IFNAMSIZ-1);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr)==-1) {
	    close(sock);
	    goto MAKESTRING;
	}

	close(sock);
	mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
	memcpy(m_ap_mac, mac, sizeof(m_ap_mac));

MAKESTRING:
	m_mac = "";
	m_mac_no_hyphen = "";
	for(int i = 0; i < 6; i ++)
	{
		format fmtr("%02X");
		if(!i)
			m_mac += "-";
		fmtr % (unsigned int)m_ap_mac[i];
		m_mac += fmtr.str();
		m_mac_no_hyphen += fmtr.str();
	}
	return;
}

void ControlChannel::UpdateMemory()
{
    string line;
    long total_mem = -1;
    long free_mem = -1;
    ifstream myfile("/proc/meminfo");

    m_memory_info = "0:0";

    if (!myfile.is_open()) {
        return;
    }

    while (getline (myfile, line)) {
        split_vector_type token;
        split(token, line, is_any_of(" "), token_compress_on);
        if(token.size() < 3)
        	continue;

        if(!token[0].compare("MemTotal:") && total_mem < 0)
        	total_mem = stringToNum<long>(token[1]);
        else if(!token[0].compare("MemFree:") && free_mem < 0)
        	free_mem = stringToNum<long>(token[1]);

        if((total_mem > 0) && (free_mem > 0))
        	break;
    }
    myfile.close();

    format fmtr("%.1f:%.1f");
    fmtr % ((float)free_mem/1024.0f) % ((float)total_mem/1024.0f);

    m_memory_info = fmtr.str();

    return;
}

void ControlChannel::UpdateCpu()
{
    string line;
    long total = 0;
    long idle = 0;
    float usage = 0.0f;
    ifstream myfile("/proc/stat");

    m_cpu_usage = "0";

    if (!myfile.is_open()) {
        return;
    }

    while (getline (myfile, line)) {
        split_vector_type token;
        split(token, line, is_any_of(" "), token_compress_on);
        if(token.size() < 5)
        	continue;
        if(token[0].compare("cpu"))
        	continue;

        total += stringToNum<long>(token[1]);
        total += stringToNum<long>(token[2]);
        total += stringToNum<long>(token[3]);
        idle = stringToNum<long>(token[4]);
        total += idle;
        if(token.size() > 5)
        	total += stringToNum<long>(token[5]);
        if(token.size() > 6)
        	total += stringToNum<long>(token[6]);
        if(token.size() > 7)
        	total += stringToNum<long>(token[7]);

        if ((total > 0) && (idle >= 0) && (idle <= total))
        	usage = (total - idle) * 100.0f / total;
    }
    myfile.close();

    format fmtr("%.1f");
    fmtr % usage;

    m_cpu_usage = fmtr.str();

    return;
}

HttpOper *ControlChannel::CreateOperHello()
{
	HttpOper *hello = new HttpOper(OP_HEARTBEAT);
	HttpPara *para;

	if(!hello)
		return NULL;

	UpdateMemory();
	UpdateCpu();

	hello->AddPara(PARA_ID, m_mac);
	hello->AddPara(PARA_CFG_FLG, m_cfg_flag);
	hello->AddPara(PARA_MEM, m_memory_info);
	hello->AddPara(PARA_CPU, m_cpu_usage);
	hello->AddPara(PARA_SYS_VER, SYS_VERSION);
	hello->AddPara(PARA_MAC_CNT, m_mac_count);
	hello->AddPara(PARA_ID_CNT, m_id_count);
	hello->AddPara(PARA_STIME, m_start_time);
	hello->AddPara(PARA_ETIME, m_end_time);

	return hello;
}

HttpOper *ControlChannel::CreateOperConfig()
{
	HttpOper *config;
	HttpPara *para;
	int flag = 0;
	RunningConfigTable running_table;
	StaticConfigTable static_table;
	string sitename, siteaddr, devaddr;
	string apid, devmodel, detailid;
	float lon, lat;

	running_table.GetFlag (flag);
	if (!flag)
		return NULL;

	running_table.GetSiteName(sitename);
	running_table.GetSiteAddr(siteaddr);
	running_table.GetDevAddr(devaddr);
	running_table.GetAppLon(lon);
	running_table.GetAppLat(lat);

	static_table.GetDevId(apid);
	static_table.GetDevModel(devmodel);
	static_table.GetDetailId(detailid);
	apid += m_mac_no_hyphen;

	config = new HttpOper(OP_BASIC_INFO);
	if(!config)
		return NULL;

	config->AddPara(PARA_SITE_NAME, sitename);
	config->AddPara(PARA_SITE_ADDR, siteaddr);
	config->AddPara(PARA_AP_ID, apid);
	config->AddPara(PARA_DEV_MOD, devmodel);
	config->AddPara(PARA_AP_MAC, m_mac);
	config->AddPara(PARA_AP_LON, lon);
	config->AddPara(PARA_AP_LAT, lat);
	config->AddPara(PARA_DEV_ADDR, devaddr);
	config->AddPara(PARA_DETAIL_ID, detailid);

	return config;
}
