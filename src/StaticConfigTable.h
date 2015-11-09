#ifndef _VDR_BASE_CONFIG_TABLE_H_
#define _VDR_BASE_CONFIG_TABLE_H_

#include "SingleRowTable.h"

#define STATIC_CONFIG_DB_NAME             ("static.db")
#define STATIC_CONFIG_TABLE_NAME          ("static")


using namespace std;

typedef enum
{
    STATIC_CONFIG_DEV_ID = 0,
    STATIC_CONFIG_DEV_MODEL,
    STATIC_CONFIG_DEV_NAME,
    STATIC_CONFIG_DEV_ADDR,
    STATIC_CONFIG_DEV_DETAIL_ID,
    STATIC_CONFIG_BUTT
}ENUM_STATIC_CONFIG_COL;

class StaticConfigTable : public SingleRowTable
{
public:
	StaticConfigTable();
};

#endif
