#ifndef _HTTP_BODY_H_
#define _HTTP_BODY_H_

#include "sqlite3.h"
#include "boost/variant.hpp"
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include "Utility.h"
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <pthread.h>
#include "LogUtility.h"
#include "Utility.h"

using namespace std;

#define OP_HEARTBEAT     0
#define OP_BASIC_INFO    42

#define PARA_OP                "op"
#define PARA_ID                "id"
#define PARA_CFG_FLG           "cfg_flg"
#define PARA_MEM               "mem"
#define PARA_CPU               "cpu"
#define PARA_SYS_VER           "sys_ver"
#define PARA_MAC_CNT           "mac_cnt"
#define PARA_ID_CNT            "id_cnt"
#define PARA_STIME             "cnt_stime"
#define PARA_ETIME             "cnt_etime"

#define PARA_SITE_NAME         "site_name"
#define PARA_SITE_ADDR         "site_addr"
#define PARA_AP_ID             "ap_id"
#define PARA_DEV_MOD           "dev_mod"
#define PARA_AP_MAC            "ap_mac"
#define PARA_AP_LON            "ap_lon"
#define PARA_AP_LAT            "ap_lat"
#define PARA_DEV_ADDR          "dev_addr"
#define PARA_DETAIL_ID         "detail_id"

typedef enum
{
    HTTP_PARA_TYPE_INT,
    HTTP_PARA_TYPE_FLOAT,
    HTTP_PARA_TYPE_STRING
} EnumHttpParaType;

typedef boost::variant<int, float, string> HttpParaValue;

inline void GetHttpParaValueString(HttpParaValue &value, string &stringValue)
{
    string *ptr = boost::get<string>(&value);
    if(ptr == NULL)
    {
        return;
    }
    else
    {
        stringValue = *ptr;
        return;
    }
}

inline int GetHttpParaValueInt(HttpParaValue &value)
{
    int *ptr = boost::get<int>(&value);
    if(ptr == NULL)
    {
        return -1;
    }
    else
    {
        return *ptr;
    }
}

inline float GetHttpParaValueFloat(HttpParaValue &value)
{
    float *ptr = boost::get<float>(&value);
    if(ptr == NULL)
    {
        return -1.0;
    }
    else
    {
        return *ptr;
    }
}

// This function is used to create sql.
inline string HttpParaValueToString(EnumHttpParaType type, HttpParaValue &value)
{
    string stringValue = "";

    switch(type)
    {
        case HTTP_PARA_TYPE_INT:
            return numToString<int>(GetHttpParaValueInt(value));
        case HTTP_PARA_TYPE_FLOAT:
            return numToString<float>(GetHttpParaValueFloat(value));
        case HTTP_PARA_TYPE_STRING:
        default:
        	GetHttpParaValueString(value, stringValue);
            return stringValue;
    }
}

class HttpPara
{
public:
	HttpPara(string name, EnumHttpParaType type, HttpParaValue value)
    {
		m_name = name;
        m_type = type;
        m_value = value;
    }

	HttpPara(string name, EnumHttpParaType type, string value)
    {
		m_name = name;
        m_type = type;
        Parse(value);
    }

	string ToString();

	static HttpPara *CreatePara(string para);

	string GetName()
	{
		return m_name;
	}

	HttpParaValue &GetValue()
	{
		return m_value;
	}

private:
	int Parse(string value);

	string m_name;
	EnumHttpParaType m_type;
    HttpParaValue m_value;
};

typedef map<string, HttpPara *> HttpParaMap;

class HttpOper
{
public:
	HttpOper(int oper)
    {
		m_oper = oper;
    }

	~HttpOper()
	{
		Clear();
	}

	string ToString();

	void AddPara(string name, int value);
	void AddPara(string name, float value);
	void AddPara(string name, string value);
	void AddPara(HttpPara *para);

	static HttpOper *CreateOper(string oper);

private:
	void Clear();
	int m_oper;
	HttpParaMap m_paras;
};

class HttpBody
{
public:
	HttpBody()
	{
		;
	}

	~HttpBody();

	void AddOper(HttpOper *oper)
	{
		m_opers.push_back(oper);
	}

	string ToString();

	static HttpBody *CreateBody(string body);
private:
	vector<HttpOper *> m_opers;
};

#endif
