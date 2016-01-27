#ifndef _MAC_RECORD_H_
#define _MAC_RECORD_H_

#include <time.h>
#include "ScanRecord.h"

typedef struct
{
	unsigned char mac[6];
	unsigned short type;
	char max_signal;
	char latest_signal;
	time_t time;
	unsigned char bssid[6];	
	unsigned char essid[64];
	unsigned char channel;
	unsigned char security[2];
}MacRecord;

struct MacRecordNode
{
	struct list_head node;
	MacRecord rec;
};

#define MAC_RECORD_TABLE MacRecordTable::Instance()

#define MAC_HEAD_LENGTH       14
#define MAC_RECORD_LENGTH     118

#define NULL_MAC  (unsigned char*)"\x00\x00\x00\x00\x00\x00"
#define BROADCAST (unsigned char*)"\xFF\xFF\xFF\xFF\xFF\xFF"

class MacRecordTable : public RecordHashGenerator
{
public:
	static MacRecordTable *Instance();
	static void Destroy();

	void UpdateMacRecord(MacRecord *rec);
	unsigned char *GetRecord(int &len, int &num);

	unsigned int GetHash(struct list_head *rec, int bucket_size);
	int IsKeySame(struct list_head *rec1, struct list_head *rec2);
	void Free(struct list_head *rec);
	struct list_head *Choose(struct list_head *old_rec, struct list_head *new_rec);
	int PrepareBuffer(int number);
	void DumpRecord(struct list_head *rec);

protected:
	MacRecordTable();

private:
	ScanRecordHash m_table;
	static MacRecordTable *m_instance;
	unsigned char *m_buf;
	int m_index;

#define ENCRYPT(byte)   ((byte) ^ 0x66)

    inline void PushByte(unsigned char *buf, int &index, unsigned char byte)
    {
    	buf[index] = ENCRYPT(byte);
        index++;
    }

    inline void PushWord(unsigned char *buf, int &index, unsigned short word)
    {
    	PushByte(buf, index, (unsigned char)((word >> 8) & 0xFF));
    	PushByte(buf, index, (unsigned char)(word & 0xFF));
    }

    inline void PushLong(unsigned char *buf, int &index, unsigned long word)
    {

		PushByte(buf, index, (unsigned char)((word >> 24) & 0xFF));
        PushByte(buf, index, (unsigned char)((word >> 16) & 0xFF));
        PushByte(buf, index, (unsigned char)((word >> 8) & 0xFF));
        PushByte(buf, index, (unsigned char)(word & 0xFF));
    }
};

#endif
