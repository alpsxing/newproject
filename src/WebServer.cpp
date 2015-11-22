#include <unistd.h>
#include "Utility.h"
#include "LogUtility.h"
#include "WebServer.h"
#include "HTTPBody.h"
#include "StaticConfigTable.h"
#include "RunningConfigTable.h"
#include "ControlChannel.h"
#include <boost/network/protocol/http/server.hpp>

namespace http = boost::network::http;

struct WebServerProcessing;
typedef http::server<WebServerProcessing> server;

struct WebServerProcessing {
	void operator()(server::request const &request, server::response &response)
	{
		server::string_type ip = source(request);
		unsigned int port = request.source_port;
		std::string request_body = body(request);
		int ret;
		enum server::response::status_type status = server::response::ok;

		LogUtility::Log(LOG_LEVEL_DEBUG, "Received request: %s", request_body.c_str());

		ret = WebServer::ProcessRequest(request_body);
		if(ret < 0)
		{
			LogUtility::Log(LOG_LEVEL_WARN, "Failed process request");
			status = server::response::bad_request;
		}

		std::string result;
		WebServer::CreateResponse(result);

		response = server::response::stock_reply(status, result);
	}

	void log(...)
	{
		// do nothing
	}
};

void WebServer::Run(std::string addr, int port)
{
	LogUtility::Log(LOG_LEVEL_DEBUG, "Web server running at http://%s:%d",
					addr.c_str(), port);

	while (1)
	{
		try {
			WebServerProcessing handler;

			server::options options(handler);
			server server_(options.address(addr).port(numToString<int>(port)));
			server_.run();
		}
		catch (std::exception &e) {
			LogUtility::Log(LOG_LEVEL_ERROR, "Web server got exception");
		}

		LogUtility::Log(LOG_LEVEL_INFO, "Restart web server...");
		sleep(5);
	}
}

int WebServer::ProcessRequest(std::string request_body)
{
	HttpBody *resp;

	resp = HttpBody::CreateBody(request_body);
	if (!resp)
	{
		LogUtility::Log(LOG_LEVEL_WARN, "Invalid response");
		return -1;
	}

	LogUtility::Log(LOG_LEVEL_DEBUG, "Request body %s processed", resp->ToString().c_str());

	std::vector<HttpOper *> *opers = resp->GetOpers();

	if(opers->size() != 1)
	{
		LogUtility::Log(LOG_LEVEL_DEBUG, "Request body has too many opers(%d)", opers->size());
		delete resp;
		return -1;
	}

	HttpOper *oper = (*opers)[0];
	int op = oper->GetOper();
	if(op != OP_LOCAL)
	{
		LogUtility::Log(LOG_LEVEL_DEBUG, "Invalid op(%d)", op);
		delete resp;
		return -1;
	}

	HttpParaMap *paras = oper->GetParas();
	HttpParaMap::iterator itr = paras->begin();
	RunningConfigTable table;

	while(itr != paras->end())
	{
		HttpPara *para = (HttpPara *)itr->second;
		std::string name = para->GetName();
		if(!name.compare(PARA_CONTROL_SERVER))
		{
			std::string url = "";
			GetHttpParaValueString(para->GetValue(), url);
			if(!url.empty()) {
				table.SetUrl(url);
			}
		}
		else if(!name.compare(PARA_SITE_NAME))
		{
			std::string site = "";
			GetHttpParaValueString(para->GetValue(), site);
			if(!site.empty()) {
				table.SetSiteName(site);
			}
		}
		else if(!name.compare(PARA_SITE_ADDR))
		{
			std::string addr = "";
			GetHttpParaValueString(para->GetValue(), addr);
			if(!addr.empty()) {
				table.SetSiteAddr(addr);
			}
		}
		else if(!name.compare(PARA_DEV_ADDR))
		{
			std::string addr = "";
			GetHttpParaValueString(para->GetValue(), addr);
			if(!addr.empty()) {
				table.SetDevAddr(addr);
			}
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

		itr ++;
	}

	table.Commit();

	CONTROL_INSTANCE->RunningTableUpdated();

	delete resp;
	return 0;
}

void WebServer::CreateResponse(std::string &result)
{
	HttpOper oper(OP_LOCAL);
	HttpPara *para;
	RunningConfigTable running_table;
	StaticConfigTable static_table;
	std::string url, siteaddr, sitename, devaddr, servcode;
	float lon = 0.0f, lat = 0.0f;
	std::string instcode, devid, devmodel, detailid;

	running_table.GetUrl(url);
	running_table.GetSiteAddr(siteaddr);
	running_table.GetSiteName(sitename);
	running_table.GetDevAddr(devaddr);
	running_table.GetAppLon(lon);
	running_table.GetAppLat(lat);
	running_table.GetSrvCode(servcode);

	static_table.GetDevId(devid);
	static_table.GetDevModel(devmodel);
	static_table.GetDetailId(detailid);
	static_table.GetInstCode(instcode);

	oper.AddPara(PARA_CONTROL_SERVER, url);
	oper.AddPara(PARA_SITE_NAME, sitename);
	oper.AddPara(PARA_SITE_ADDR, siteaddr);
	oper.AddPara(PARA_DEV_ADDR, devaddr);
	oper.AddPara(PARA_SRV_CODE, servcode);
	oper.AddPara(PARA_AP_LON, lon);
	oper.AddPara(PARA_AP_LAT, lat);
	oper.AddPara(PARA_AP_ID, devid);
	oper.AddPara(PARA_DEV_MOD, devmodel);
	oper.AddPara(PARA_DETAIL_ID, detailid);
	oper.AddPara("inst_code", instcode);
	oper.AddPara(PARA_ID, CONTROL_INSTANCE->GetMac());

	result = oper.ToString();

	return;
}
