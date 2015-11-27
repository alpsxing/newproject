#ifndef _MAC_RECORD_H_
#define _MAC_RECORD_H_

#include <time.h>
#include "ScanRecord.h"

typedef struct
{
	unsigned char mac[6];
	unsigned short type;
	unsigned char max_signal;
	unsigned char latest_signal;
	time_t time;
}MacRecord;

struct MacRecordNode
{
	struct list_head node;
	MacRecord rec;
};

#define MAC_RECORD_TABLE MacRecordTable::Instance()

class MacRecordTable : public RecordHashGenerator
{
public:
	static MacRecordTable *Instance();
	static void Destroy();

	void UpdateMacRecord(MacRecord *rec);

	unsigned int GetHash(struct list_head *rec, int bucket_size);
	int IsKeySame(struct list_head *rec1, struct list_head *rec2);
	void Free(struct list_head *rec);
	struct list_head *Choose(struct list_head *old_rec, struct list_head *new_rec);

protected:
	MacRecordTable();

private:
	ScanRecordHash m_table;
	static MacRecordTable *m_instance;
};

#endif
