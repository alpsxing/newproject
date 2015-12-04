#include "RunningConfigTable.h"

const static SQLiteColumn VDRRunningConfigCols[RUNNING_CONFIG_BUTT] = {
//                Head            Type                             not   no     auto   prim   Unique Default
//                                                                 Null  Case   Incre  Key           Value
    SQLiteColumn("SiteID",        ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("SiteName",      ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("SiteAddr",      ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("ApLon",         ENUM_SQLite_COLUMN_FLOAT,        true, false, false, false, false, SQLiteValue(0.0f)),
    SQLiteColumn("ApLat",         ENUM_SQLite_COLUMN_FLOAT,        true, false, false, false, false, SQLiteValue(0.0f)),
    SQLiteColumn("DevAddr",       ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("URL",           ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("192.168.1.10:7000")),
    SQLiteColumn("Invl",          ENUM_SQLite_COLUMN_INTEGER,      true, false, false, false, false, SQLiteValue(30)),
    SQLiteColumn("UpInvl",        ENUM_SQLite_COLUMN_INTEGER,      true, false, false, false, false, SQLiteValue(300)),
    SQLiteColumn("SrvCode",       ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("MyCode")),
    SQLiteColumn("DataSrv",       ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("127.0.0.1:6000")),
    SQLiteColumn("Flag",          ENUM_SQLite_COLUMN_INTEGER,      true, false, false, false, false, SQLiteValue(1)),
};

RunningConfigTable::RunningConfigTable():
    SingleRowTable(RUNNING_CONFIG_DB_NAME,
    			   RUNNING_CONFIG_TABLE_NAME, 1, (int)RUNNING_CONFIG_BUTT, VDRRunningConfigCols)
{
}
