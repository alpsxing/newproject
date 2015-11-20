#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <vector>
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"
#include "Utility.h"
#include "version.h"
#include "RunningConfigTable.h"
#include "StaticConfigTable.h"
#include "ControlChannel.h"
#include <boost/network/protocol/http/client.hpp>

using boost::format;
using namespace boost::network;
using namespace boost;

typedef std::vector<std::string> split_vector_type;

ControlChannel *ControlChannel::m_instance = NULL;

static void ControlChannelTimerHandler(std::string &name, void *data, void *data2)
{
	ControlChannel *handler = (ControlChannel *)data;
    if(handler != NULL)
    {
        handler->HandleTimer(name, data2);
    }
}

ControlChannel *ControlChannel::Instance()
{
    if(!m_instance)
    {
        m_instance = new ControlChannel();
    }

    return m_instance;
}

void ControlChannel::Destroy()
{
    if(m_instance != NULL)
    {
        delete m_instance;
    }
}

ControlChannel::ControlChannel()
{
	RunningConfigTable running_table;
	int invl = 300;

	m_cfg_flag = 0;
	UpdateMac("eth0");
	m_mac_count = 0;
	m_id_count = 0;
	m_start_time = "000000000000";
	m_end_time = "000000000000";
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init (&m_cond, NULL);
	m_should_send = 0;
	m_runtable_flag = 0;
	m_url = "";
	running_table.GetFlag(m_runtable_flag);
	running_table.GetUrl(m_url);
	running_table.GetInvl(invl);
	if((invl < 10) || (invl > 3600))
		invl = 300;
	m_helloTimer = NULL;
	ChangeHelloInvl(invl, 0);
	m_exit = 0;
}

ControlChannel::~ControlChannel()
{
    DestroyTimer(m_helloTimer);
    Stop();
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

void ControlChannel::ChangeHelloInvl(int invl, int start)
{
	if(m_helloTimer)
		DestroyTimer(m_helloTimer);
	m_helloTimer = CreateTimer(HELLO_TIMER_NAME, TIMER_SEC_TO_MSEC(invl), this, ControlChannelTimerHandler, true);
	if(start)
		StartTimer(m_helloTimer);
}

void ControlChannel::Start()
{
    if(m_exit == true)
    {
        m_exit = false;
        pthread_create(&m_tid, NULL, this->ThreadLoop, this);
    }

    return;
}

void ControlChannel::Stop()
{
    if(m_exit != true)
    {
    	pthread_mutex_lock(&m_mutex);
        m_exit = true;
        pthread_cond_signal(&m_cond);
        pthread_mutex_unlock(&m_mutex);
        pthread_join(m_tid, NULL);
    }

    return;
}

void *ControlChannel::ThreadLoop(void *arg)
{
	ControlChannel *controlChannel = reinterpret_cast<ControlChannel *> (arg);

	StartTimer(controlChannel->m_helloTimer);

	controlChannel->SendRequest(1);
	while(1)
	{
		pthread_mutex_lock(&controlChannel->m_mutex);
		if(!controlChannel -> m_exit && !controlChannel->m_should_send)
			pthread_cond_wait(&controlChannel->m_cond, &controlChannel->m_mutex);
		if(controlChannel->m_exit)
		{
			pthread_mutex_unlock(&controlChannel->m_mutex);
			break;
		}

		if(controlChannel->m_should_send)
		{
			controlChannel->m_should_send = 0;
			pthread_mutex_unlock(&controlChannel->m_mutex);
			controlChannel->SendRequest(0);
		}
		else
			pthread_mutex_unlock(&controlChannel->m_mutex);
	}
	StopTimer(controlChannel->m_helloTimer);
	pthread_exit(NULL);
}

void ControlChannel::SendRequest(int first)
{
	HttpOper *hello = CreateOperHello();
	HttpOper *config = NULL;
	HttpBody http_body;
	unsigned short response_status;
	std::string response_body;
	HttpBody *resp;

	if(!hello)
		return;
	if(first)
		config = CreateOperConfig();
	http_body.AddOper(hello);
	if(!config)
		http_body.AddOper(config);

	http::client::options options;
	options.timeout(10);
	http::client client(options);
	try
	{
		http::client::request request(m_url);
		request << header("Content-Type", "application/x-www-form-urlencoded");
		request << body(http_body.ToString());
		http::client::response response = client.post(request);
		response_status = http::status(response);
		if(response_status != 200)
			return;
		response_body = body(response);
	} catch (std::exception &e)
	{
		return;
	}

	resp = HttpBody::CreateBody(response_body);
	if(!resp)
		return;

	return;
}

void ControlChannel::ProcessResponse(HttpBody *resp)
{
	std::vector<HttpOper *> *opers = resp->GetOpers();

	for(int i = 0; i < opers->size(); i ++)
	{
		HttpOper *oper = (*opers)[i];
		int op = oper->GetOper();
		switch(op)
		{
		case OP_HEARTBEAT:
			ProcessHelloResponse(oper);
			break;
		case OP_BASIC_INFO:
			ProcessConfigResponse(oper);
			break;
		}
	}
}

void ControlChannel::ProcessHelloResponse(HttpOper *oper)
{
	HttpParaMap *paras = oper->GetParas();
	HttpParaMap::iterator itr = paras->begin();

	while(itr != paras->end())
	{
		HttpPara *para = (HttpPara *)itr->second;
		std::string name = para->GetName();
		if(!name.compare(PARA_CFG_FLG))
		{
			m_cfg_flag = GetHttpParaValueInt(para->GetValue());
		}
		else if(!name.compare(PARA_CFG_FLG))
		{
			int invl = GetHttpParaValueInt(para->GetValue());
			if((invl > 10) && (invl < 3600))
			{
				RunningConfigTable table;
				table.SetInvl(invl);
				table.Commit();
				ChangeHelloInvl(invl, 1);
			}
		}
	}

	return;
}

void ControlChannel::ProcessConfigResponse(HttpOper *oper)
{
	HttpParaMap *paras = oper->GetParas();
	HttpParaMap::iterator itr = paras->begin();
	RunningConfigTable table;

	while(itr != paras->end())
	{
		HttpPara *para = (HttpPara *)itr->second;
		std::string name = para->GetName();
		if(!name.compare(PARA_SITE_ID))
		{
			std::string siteid = "";
			GetHttpParaValueString(para->GetValue(), siteid);
			if(!siteid.empty())
				table.SetSiteId(siteid);
		}
		else if(!name.compare(PARA_SITE_NAME))
		{
			std::string sitename = "";
			GetHttpParaValueString(para->GetValue(), sitename);
			if(!sitename.empty())
				table.SetSiteName(sitename);
		}
		else if(!name.compare(PARA_SITE_ADDR))
		{
			std::string siteaddr = "";
			GetHttpParaValueString(para->GetValue(), siteaddr);
			if(!siteaddr.empty())
				table.SetSiteAddr(siteaddr);
		}
		else if(!name.compare(PARA_AP_LON))
		{
			float lon = GetHttpParaValueFloat(para->GetValue());
			table.SetAppLon(lon);
		}
		else if(!name.compare(PARA_AP_LAT))
		{
			float lat = GetHttpParaValueFloat(para->GetValue());
			table.SetAppLat(lat);
		}
		else if(!name.compare(PARA_DEV_ADDR))
		{
			std::string devaddr = "";
			GetHttpParaValueString(para->GetValue(), devaddr);
			if(!devaddr.empty())
				table.SetDevAddr(devaddr);
		}
	}

	table.Commit();

	return;
}

void ControlChannel::UpdateMac(std::string dev_name)
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
    std::string line;
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
    std::string line;
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
	RunningConfigTable running_table;
	StaticConfigTable static_table;
	std::string sitename, siteaddr, devaddr;
	std::string apid, devmodel, detailid;
	float lon, lat;

	if (!m_runtable_flag)
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

void ControlChannel::HandleTimer(std::string &name, void *data)
{
	if(name == HELLO_TIMER_NAME)
	{
	    if(!m_should_send)
	    {
	    	pthread_mutex_lock(&m_mutex);
	    	m_should_send = 1;
	        pthread_cond_signal(&m_cond);
	        pthread_mutex_unlock(&m_mutex);
	    }
	}
}
