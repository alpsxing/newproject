#include <string.h>
#include <stdlib.h>
#include "MacRecord.h"
#include "LogUtility.h"
#include "ControlChannel.h"

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
	m_buf = NULL;
	m_index = 0;
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

int MacRecordTable::PrepareBuffer(int number)
{
	int index;
	unsigned char mac[6];
	if(number <= 0)
		return -1;

	if(m_buf)
	{
		free(m_buf);
        m_buf = NULL;
    }

	m_buf = (unsigned char *)malloc(MAC_HEAD_LENGTH + MAC_RECORD_LENGTH * number);
	if(!m_buf)
		return -1;
	m_index = 0;
	PushWord(m_buf, m_index, 0x01);
	CONTROL_INSTANCE->GetMac(mac);
	for(int i = 0; i < 6; i ++)
		PushByte(m_buf, m_index, mac[i]);
	PushWord(m_buf, m_index, 0x01);
	PushLong(m_buf, m_index, MAC_RECORD_LENGTH * number);
    return 0;
}

void MacRecordTable::DumpRecord(struct list_head *rec)
{
    int ilen=0, i=0;
    string ch;
        
	struct MacRecordNode *macRecord = list_entry(rec, struct MacRecordNode, node);
	PushWord(m_buf, m_index, 0x00);
	PushWord(m_buf, m_index, macRecord->rec.type);
	for(int i = 0; i < 6; i ++)
		PushByte(m_buf, m_index, macRecord->rec.mac[i]);
	PushByte(m_buf, m_index, macRecord->rec.max_signal);
	PushByte(m_buf, m_index, macRecord->rec.latest_signal);
	PushLong(m_buf, m_index, (unsigned int)macRecord->rec.time);
    if(macRecord->rec.type == 2)
    {
        ilen = strlen( macRecord->rec.essid);
        if(ilen > 0)
        {
            PushWord(m_buf, m_index, 0x06);
            PushWord(m_buf, m_index, ilen);
            for( i = 0; i < ilen; i ++)
                PushByte(m_buf, m_index, macRecord->rec.essid[i]);
        }
        PushWord(m_buf, m_index, 0x07);
        PushWord(m_buf, m_index, 12);
        for(i = 0; i < 6; i ++)
        {
            ch = hexToString(macRecord->rec.bssid[i]);
            PushByte(m_buf, m_index, ch[0]);
            PushByte(m_buf, m_index, ch[1]);
        }
        
        PushWord(m_buf, m_index, 0x08);
        PushWord(m_buf, m_index, 2);
        ch = numToString(macRecord->rec.channel);
        PushByte(m_buf, m_index, ch[0]);
        PushByte(m_buf, m_index, ch[1]);
        PushWord(m_buf, m_index, 0x09);
        PushWord(m_buf, m_index, 2);
		PushByte(m_buf, m_index, macRecord->rec.security[0]);
		PushByte(m_buf, m_index, macRecord->rec.security[1]);
		
    }
    else if(macRecord->rec.type == 1)
    {
        ilen = strlen( macRecord->rec.essid);
        if(ilen > 0)
        {
            PushWord(m_buf, m_index, 0x06);
            PushWord(m_buf, m_index, ilen);
            for( i = 0; i < ilen; i ++)
                PushByte(m_buf, m_index, macRecord->rec.essid[i]);
        }
        if(memcmp( macRecord->rec.bssid, BROADCAST, 6 ) !=0  && memcmp( macRecord->rec.bssid, NULL_MAC, 6 ) !=0)
        {
            PushWord(m_buf, m_index, 0x07);
            PushWord(m_buf, m_index, 12);
            for(i = 0; i < 6; i ++)
            {
                ch = hexToString(macRecord->rec.bssid[i]);
                PushByte(m_buf, m_index, ch[0]);
                PushByte(m_buf, m_index, ch[1]);
            }
            PushWord(m_buf, m_index, 0x08);
            PushWord(m_buf, m_index, 2);
            ch = numToString(macRecord->rec.channel);
            PushByte(m_buf, m_index, ch[0]);
            PushByte(m_buf, m_index, ch[1]);         
        }

    }
    LogUtility::Log(LOG_LEVEL_DEBUG, "type=%d,%02x:%02x:%02x:%02x:%02x:%02x, %d, %d, %d",macRecord->rec.type,macRecord->rec.mac[0],macRecord->rec.mac[1],macRecord->rec.mac[2],
        macRecord->rec.mac[3],macRecord->rec.mac[4],macRecord->rec.mac[5],macRecord->rec.max_signal,macRecord->rec.latest_signal,(int)macRecord->rec.time);
}

unsigned char *MacRecordTable::GetRecord(int &len, int &num)
{
    m_index = 0;
	num = m_table.DumpRecord(1);
	len = m_index;
    if(m_index > 14)
    {
        int d_len = m_index - 14;
        m_buf[10]= ENCRYPT((d_len >> 24)&0xFF);
        m_buf[11]= ENCRYPT((d_len >> 16)&0xFF);
        m_buf[12]= ENCRYPT((d_len >> 8)&0xFF);
        m_buf[13]= ENCRYPT(d_len & 0xFF);        
    }
	return m_buf;
}
