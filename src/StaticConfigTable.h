#ifndef _STATIC_CONFIG_TABLE_H_
#define _STATIC_CONFIG_TABLE_H_

#include "SingleRowTable.h"

#define STATIC_CONFIG_DB_NAME             ("static.db")
#define STATIC_CONFIG_TABLE_NAME          ("static")


using namespace std;

typedef enum
{
    STATIC_CONFIG_DEV_ID = 0,
    STATIC_CONFIG_DEV_MODEL,
    STATIC_CONFIG_DEV_DETAIL_ID,
	STATIC_CONFIG_DEV_INST_CODE,
    STATIC_CONFIG_BUTT
}ENUM_STATIC_CONFIG_COL;

class StaticConfigTable : public SingleRowTable
{
public:
	StaticConfigTable();

	int GetDevId(string &id)
	{
		return GetStringValue(STATIC_CONFIG_DEV_ID, id);
	}
	int SetDevId(string &id)
	{
		return SetStringValue(STATIC_CONFIG_DEV_ID, id);
	}
	int GetDevModel(string &model)
	{
		return GetStringValue(STATIC_CONFIG_DEV_MODEL, model);
	}
	int SetDevModel(string &model)
	{
		return SetStringValue(STATIC_CONFIG_DEV_MODEL, model);
	}
	int GetDetailId(string &id)
	{
		return GetStringValue(STATIC_CONFIG_DEV_DETAIL_ID, id);
	}
	int SetDetailId(string &id)
	{
		return SetStringValue(STATIC_CONFIG_DEV_DETAIL_ID, id);
	}
	int GetInstCode(string &code)
	{
		return GetStringValue(STATIC_CONFIG_DEV_INST_CODE, code);
	}
	int SetInstCode(string &code)
	{
		return SetStringValue(STATIC_CONFIG_DEV_INST_CODE, code);
	}
};

#endif
