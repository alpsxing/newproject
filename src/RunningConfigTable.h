#ifndef _RUNNING_CONFIG_TABLE_H_
#define _RUNNING_CONFIG_TABLE_H_

#include "SingleRowTable.h"

#define RUNNING_CONFIG_DB_NAME             ("running.db")
#define RUNNING_CONFIG_TABLE_NAME          ("running")


using namespace std;

typedef enum
{
    RUNNING_CONFIG_SITE_ID = 0,
    RUNNING_CONFIG_SITE_NAME,
    RUNNING_CONFIG_SITE_ADDR,
    RUNNING_CONFIG_AP_LON,
    RUNNING_CONFIG_AP_LAT,
	RUNNING_CONFIG_DEV_ADDR,
	RUNNING_CONFIG_URL,
	RUNNING_CONFIG_INVL,
	RUNNING_CONFIG_UP_INVL,
	RUNNING_CONFIG_SRV_CODE,
	RUNNING_CONFIG_DATA_SERVER,
	RUNNING_CONFIG_FLAG,
    RUNNING_CONFIG_BUTT
}ENUM_RUNNING_CONFIG_COL;

class RunningConfigTable : public SingleRowTable
{
public:
	RunningConfigTable();

	int GetInvl(int &invl)
	{
		return GetIntValue(RUNNING_CONFIG_INVL, invl);
	}
	int SetInvl(int invl)
	{
		return SetIntValue(RUNNING_CONFIG_INVL, invl);
	}
	int GetUpInvl(int &invl)
	{
		return GetIntValue(RUNNING_CONFIG_UP_INVL, invl);
	}
	int SetUpInvl(int invl)
	{
		return SetIntValue(RUNNING_CONFIG_UP_INVL, invl);
	}
	int SetSiteId(string &siteid)
	{
		return SetStringValue(RUNNING_CONFIG_SITE_ID, siteid);
	}
	int GetFlag(int &flag)
	{
		return GetIntValue(RUNNING_CONFIG_FLAG, flag);
	}
	int SetFlag(int flag)
	{
		return SetIntValue(RUNNING_CONFIG_FLAG, flag);
	}
	int GetSiteName(string &name)
	{
		return GetStringValue(RUNNING_CONFIG_SITE_NAME, name);
	}
	int SetSiteName(string &name)
	{
		return SetStringValue(RUNNING_CONFIG_SITE_NAME, name);
	}
	int GetSiteAddr(string &addr)
	{
		return GetStringValue(RUNNING_CONFIG_SITE_ADDR, addr);
	}
	int SetSiteAddr(string &addr)
	{
		return SetStringValue(RUNNING_CONFIG_SITE_ADDR, addr);
	}
	int GetAppLon(float &lon)
	{
		return GetFloatValue(RUNNING_CONFIG_AP_LON, lon);
	}
	int SetAppLon(float lon)
	{
		return SetFloatValue(RUNNING_CONFIG_AP_LON, lon);
	}
	int GetAppLat(float &lat)
	{
		return GetFloatValue(RUNNING_CONFIG_AP_LAT, lat);
	}
	int SetAppLat(float lat)
	{
		return SetFloatValue(RUNNING_CONFIG_AP_LAT, lat);
	}
	int GetDevAddr(string &addr)
	{
		return GetStringValue(RUNNING_CONFIG_DEV_ADDR, addr);
	}
	int SetDevAddr(string &addr)
	{
		return SetStringValue(RUNNING_CONFIG_DEV_ADDR, addr);
	}
	int GetUrl(string &url)
	{
		return GetStringValue(RUNNING_CONFIG_URL, url);
	}
	int SetUrl(string &url)
	{
		return SetStringValue(RUNNING_CONFIG_URL, url);
	}
	int GetSrvCode(string &code)
	{
		return GetStringValue(RUNNING_CONFIG_SRV_CODE, code);
	}
	int SetSrvCode(string &code)
	{
		return SetStringValue(RUNNING_CONFIG_SRV_CODE, code);
	}
	int GetDataUrl(string &url)
	{
		return GetStringValue(RUNNING_CONFIG_DATA_SERVER, url);
	}
	int SetDataUrl(string &url)
	{
		return SetStringValue(RUNNING_CONFIG_DATA_SERVER, url);
	}
};

#endif
