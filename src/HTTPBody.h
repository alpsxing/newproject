#ifndef _SQLITE_TABLE_H_
#define _SQLITE_TABLE_H_

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

	HttpPara(string name, EnumHttpParaType type)
    {
		m_name = name;
        m_type = type;
    }

	string ToString();

	int Parse(string value);

private:

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

private:
	void Clear();
	int m_oper;
	HttpParaMap m_paras;
};

#endif
