#ifndef _SCAN_RECORD_H_
#define _SCAN_RECORD_H_

#include <pthread.h>
#include <vector>
#include "ulist.h"

class RecordHashGenerator
{
public:
	virtual unsigned int GetHash(struct list_head *rec, int bucket_size) = 0;
	virtual int IsKeySame(struct list_head *rec1, struct list_head *rec2) = 0;
	virtual void Free(struct list_head *rec) = 0;
	virtual struct list_head *Choose(struct list_head *old_rec, struct list_head *new_rec) = 0;
	virtual int PrepareBuffer(int number) = 0;
	virtual void DumpRecord(struct list_head *rec) = 0;
};

class ScanRecordHash
{
public:
	ScanRecordHash(RecordHashGenerator *hashkey, unsigned int bucket_size = 256);
	~ScanRecordHash();
	int AddRecord(struct list_head *rec);
	void DumpRecord(int clean);

protected:
	void Clear();

private:
	void DeleteRecord(struct list_head *rec);

	std::vector<struct list_head> m_table;
	RecordHashGenerator *m_hashkey;
	pthread_mutex_t m_mutex;
	int m_number;
};

#endif
