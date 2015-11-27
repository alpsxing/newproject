#include <string.h>
#include <stdlib.h>
#include "MacRecord.h"
#include "LogUtility.h"

MacRecordTable *MacRecordTable::m_instance = NULL;

MacRecordTable *MacRecordTable::Instance()
{
    if(!m_instance)
    {
        m_instance = new MacRecordTable();
    }

    return m_instance;
}

void MacRecordTable::Destroy()
{
    if(m_instance != NULL)
    {
        delete m_instance;
    }
}

MacRecordTable::MacRecordTable()
	: m_table(this)
{
	;
}

unsigned int MacRecordTable::GetHash(struct list_head *rec, int bucket_size)
{
	struct MacRecordNode *macRecord = list_entry(rec, struct MacRecordNode, node);
	unsigned hashkey = 0;

	for(int i = 0; i < sizeof(macRecord->rec.mac); i ++)
	{
		hashkey += macRecord->rec.mac[i];
	}

	return (hashkey % bucket_size);
}

int MacRecordTable::IsKeySame(struct list_head *rec1, struct list_head *rec2)
{
	struct MacRecordNode *macRecord1 = list_entry(rec1, struct MacRecordNode, node);
	struct MacRecordNode *macRecord2 = list_entry(rec2, struct MacRecordNode, node);

	return !memcmp(macRecord1->rec.mac, macRecord2->rec.mac, sizeof(macRecord1->rec.mac));
}

void MacRecordTable::Free(struct list_head *rec)
{
	struct MacRecordNode *macRecord = list_entry(rec, struct MacRecordNode, node);
	free(macRecord);
}

struct list_head *MacRecordTable::Choose(struct list_head *old_rec, struct list_head *new_rec)
{
	struct MacRecordNode *macRecord_old = list_entry(old_rec, struct MacRecordNode, node);
	struct MacRecordNode *macRecord_new = list_entry(new_rec, struct MacRecordNode, node);

	if(macRecord_old->rec.max_signal > macRecord_new->rec.max_signal)
		macRecord_new->rec.max_signal = macRecord_old->rec.max_signal;

	return new_rec;
}

void MacRecordTable::UpdateMacRecord(MacRecord *rec)
{
	if (!rec)
	{
		LogUtility::Log(LOG_LEVEL_ERROR, "UpdateMacRecord::MacRecordTable rec is NULL\n");
		return;
	}

	struct MacRecordNode *macRecord = (struct MacRecordNode *)malloc(sizeof(struct MacRecordNode));
	if(!macRecord)
	{
		LogUtility::Log(LOG_LEVEL_ERROR, "MacRecordTable::UpdateMacRecord malloc failed\n");
		return;
	}

	memcpy(&macRecord->rec, rec, sizeof(MacRecord));
	INIT_LIST_HEAD(&macRecord->node);
	m_table.AddRecord(&macRecord->node);
}
