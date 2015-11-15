#include "StaticConfigTable.h"

const static SQLiteColumn VDRStaticConfigCols[STATIC_CONFIG_BUTT] = {
//                Head            Type                             not   no     auto   prim   Unique Default
//                                                                 Null  Case   Incre  Key           Value
    SQLiteColumn("DevID",         ENUM_SQLite_COLUMN_STRING,       true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("DevModel",      ENUM_SQLite_COLUMN_INTEGER,      true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("DevName",       ENUM_SQLite_COLUMN_INTEGER,      true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("DevAddr",       ENUM_SQLite_COLUMN_INTEGER,      true, false, false, false, false, SQLiteValue("NULL")),
    SQLiteColumn("DevDetailID",   ENUM_SQLite_COLUMN_INTEGER,      true, false, false, false, false, SQLiteValue("NULL")),
};

StaticConfigTable::StaticConfigTable():
    SingleRowTable(STATIC_CONFIG_DB_NAME,
                   STATIC_CONFIG_TABLE_NAME, 1, (int)STATIC_CONFIG_BUTT, VDRStaticConfigCols)
{
}
