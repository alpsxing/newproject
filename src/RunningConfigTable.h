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
	RUNNING_CONFIG_FLAG,
    RUNNING_CONFIG_BUTT
}ENUM_RUNNING_CONFIG_COL;

class RunningConfigTable : public SingleRowTable
{
public:
	RunningConfigTable();

	int GetFlag(int &flag)
	{
		return GetIntValue(RUNNING_CONFIG_FLAG, flag);
	}

	int GetSiteName(string &name)
	{
		return GetStringValue(RUNNING_CONFIG_SITE_NAME, name);
	}
	int GetSiteAddr(string &addr)
	{
		return GetStringValue(RUNNING_CONFIG_SITE_ADDR, addr);
	}
	int GetAppLon(float &lon)
	{
		return GetFloatValue(RUNNING_CONFIG_AP_LON, lon);
	}
	int GetAppLat(float &lat)
	{
		return GetFloatValue(RUNNING_CONFIG_AP_LAT, lat);
	}
	int GetDevAddr(string &addr)
	{
		return GetStringValue(RUNNING_CONFIG_DEV_ADDR, addr);
	}
	int GetUrl(string &url)
	{
		return GetStringValue(RUNNING_CONFIG_URL, url);
	}
};

#endif
