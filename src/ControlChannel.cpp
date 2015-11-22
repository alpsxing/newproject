#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
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
	m_cfg_flag = 0;
	UpdateMac("eth0");
	m_mac_count = 0;
	m_id_count = 0;
	m_start_time = "000000000000";
	m_end_time = "000000000000";

	m_should_send = 0;
	m_should_update = 0;
	m_send_base_info = 0;

	m_runtable_flag = 0;
	m_invl = 300;
	UpdateRunningParas();
	UpdateStaticParas();

	m_helloTimer = NULL;
	ChangeHelloInvl(m_invl, 0);
	m_exit = 0;

	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init (&m_cond, NULL);
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
    if(m_exit)
        m_exit = false;
    else
        pthread_create(&m_tid, NULL, this->ThreadLoop, this);

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

	LogUtility::Log(LOG_LEVEL_DEBUG, "ControlChannel starting...");

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

		if(controlChannel->m_should_update)
		{
			controlChannel->m_should_update = 0;
			controlChannel->UpdateRunningParas();
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
	HttpOper *base_info = NULL;
	HttpOper *config = NULL;
	HttpBody http_body;
	unsigned short response_status;
	std::string response_body;
	HttpBody *resp;

	LogUtility::Log(LOG_LEVEL_DEBUG, "ControlChannel send request...");

	if(!hello)
	{
		LogUtility::Log(LOG_LEVEL_ERROR, "ControlChannel failed to create hello oper.");
		return;
	}
	if(first || m_send_base_info)
	{
		LogUtility::Log(LOG_LEVEL_DEBUG, "ControlChannel create base info oper");
		base_info = CreateOperBaseInfo();
	}
	if(m_configResp.size())
	{
		LogUtility::Log(LOG_LEVEL_DEBUG, "ControlChannel create dev config oper");
		config = CreateOperConfig();
	}
	http_body.AddOper(hello);
	if(base_info)
		http_body.AddOper(base_info);
	if(config)
		http_body.AddOper(config);


	http::client::options options;
	options.timeout(10);
	http::client client(options);
	try
	{
		std::string body_string = http_body.ToString();
		http::client::request request(m_url);
		LogUtility::Log(LOG_LEVEL_DEBUG, "ControlChannel sending to %s %s", m_url.c_str(), body_string.c_str());
		request << header("Content-Type", "application/x-www-form-urlencoded");
		request << header("Content-Length", numToString<unsigned int>((unsigned int)body_string.length()));
		request << body(http_body.ToString());
		http::client::response response = client.post(request);
		response_status = http::status(response);
		if(response_status != 200)
		{
			LogUtility::Log(LOG_LEVEL_WARN, "ControlChannel response status(%u) is not ok", response_status);
			return;
		}
		response_body = body(response);
		m_send_base_info= 0;
	} catch (std::exception &e)
	{
		LogUtility::Log(LOG_LEVEL_WARN, "ControlChannel get response failed with exception");
		return;
	}

	resp = HttpBody::CreateBody(response_body);
	if(!resp)
	{
		LogUtility::Log(LOG_LEVEL_WARN, "ControlChannel response parsed failed");
		return;
	}

	ProcessResponse(resp);

	delete resp;

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
			ProcessBaseInfoResponse(oper);
			break;
		case OP_DEVICE_CONFIG:
			ProcessDeviceConfigResponse(oper);
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
		else if(!name.compare(PARA_INVL))
		{
			int invl = GetHttpParaValueInt(para->GetValue());
			if((invl > 10) && (invl < 3600))
			{
				RunningConfigTable table;
				table.SetInvl(invl);
				table.Commit();
				m_invl = invl;
				ChangeHelloInvl(invl, 1);
			}
		}
		itr ++;
	}

	return;
}

void ControlChannel::ProcessBaseInfoResponse(HttpOper *oper)
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
		itr ++;
	}

	m_runtable_flag = 1;
	table.SetFlag(m_runtable_flag);
	table.Commit();
	m_should_update = 1;

	return;
}

void ControlChannel::ProcessDeviceConfigResponse(HttpOper *oper)
{
	HttpParaMap *paras = oper->GetParas();
	HttpParaMap::iterator itr = paras->begin();
	RunningConfigTable table;

	m_configResp.clear();

	while(itr != paras->end())
	{
		HttpPara *para = (HttpPara *)itr->second;
		std::string name = para->GetName();
		if(!name.compare(PARA_UP_INVL))
		{
			int invl = GetHttpParaValueInt(para->GetValue());
			if((invl > 10) && (invl < 3600))
			{
				RunningConfigTable table;
				table.SetUpInvl(invl);
				table.Commit();
			}
		}
		else if(!name.compare(PARA_SRV_CODE))
		{
			std::string code = "";
			GetHttpParaValueString(para->GetValue(), code);
			if(!code.empty())
				table.SetSrvCode(code);
		}
		else if(!name.compare(PARA_DATA_SERVER))
		{
			std::string url = "";
			GetHttpParaValueString(para->GetValue(), url);
			if(!url.empty())
				table.SetDataUrl(url);
		}
		else if(!name.compare(PARA_CONTROL_SERVER))
		{
			std::string url = "";
			GetHttpParaValueString(para->GetValue(), url);
			if(!url.empty()) {
				table.SetUrl(url);
				m_url = url;
			}
		}

		m_configResp.insert(std::map<std::string, int>::value_type(name, 1));
		itr ++;
	}

	table.Commit();

	m_should_update = 1;

	return;
}

void ControlChannel::UpdateMac(std::string dev_name)
{
	struct ifreq ifr;
	int sock;
	const unsigned char* mac;

	LogUtility::Log(LOG_LEVEL_DEBUG, "Update mac...");

	memset(m_ap_mac, 0, sizeof(m_ap_mac));

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
	{
		LogUtility::Log(LOG_LEVEL_ERROR, "Fail to create socket %d.", errno);
		goto MAKESTRING;
	}
	strncpy(ifr.ifr_name, dev_name.c_str(), IFNAMSIZ-1);

	if (ioctl(sock, SIOCGIFHWADDR, &ifr)==-1) {
	    close(sock);
		LogUtility::Log(LOG_LEVEL_ERROR, "Fail to ioctl.");
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
		if(i)
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

HttpOper *ControlChannel::CreateOperBaseInfo()
{
	HttpOper *base_info;
	HttpPara *para;

	if (!m_runtable_flag)
		return NULL;

	base_info = new HttpOper(OP_BASIC_INFO);
	if(!base_info)
		return NULL;

	base_info->AddPara(PARA_SITE_NAME, m_sitename);
	base_info->AddPara(PARA_SITE_ADDR, m_siteaddr);
	base_info->AddPara(PARA_AP_ID, m_apid);
	base_info->AddPara(PARA_DEV_MOD, m_devmodel);
	base_info->AddPara(PARA_AP_MAC, m_mac);
	base_info->AddPara(PARA_AP_LON, m_lon);
	base_info->AddPara(PARA_AP_LAT, m_lat);
	base_info->AddPara(PARA_DEV_ADDR, m_devaddr);
	base_info->AddPara(PARA_DETAIL_ID, m_detailid);

	return base_info;
}

HttpOper *ControlChannel::CreateOperConfig()
{
	HttpOper *config = NULL;
	HttpPara *para;
	std::map<std::string, int>::iterator itr = m_configResp.begin();


	while(itr != m_configResp.end())
	{
		std::string name = (std::string)itr->first;
		int value = (int)itr->second;
		if(!config) {
			config = new HttpOper(OP_DEVICE_CONFIG);
			if(!config)
				break;
		}
		config->AddPara(name, value);
		itr ++;
	}

	m_configResp.clear();

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

void ControlChannel::UpdateRunningParas()
{
	RunningConfigTable running_table;

	running_table.GetFlag(m_runtable_flag);
	running_table.GetUrl(m_url);
	running_table.GetSiteName(m_sitename);
	running_table.GetSiteAddr(m_siteaddr);
	running_table.GetDevAddr(m_devaddr);
	running_table.GetAppLon(m_lon);
	running_table.GetAppLat(m_lat);
	running_table.GetInvl(m_invl);

	trim(m_url);
	if(m_url.find("http://") == std::string::npos)
		m_url = "http://" + m_url;
	if((m_invl < 10) || (m_invl > 3600))
		m_invl = 300;

	return;
}

void ControlChannel::UpdateStaticParas()
{
	StaticConfigTable static_table;

	static_table.GetDevId(m_apid);
	static_table.GetDevModel(m_devmodel);
	static_table.GetDetailId(m_detailid);
	m_apid += m_mac_no_hyphen;
}

void ControlChannel::RunningTableUpdated()
{
	m_should_update = 1;
	m_send_base_info = 1;
}
