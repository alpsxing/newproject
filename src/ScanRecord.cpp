#include "ScanRecord.h"
#include "LogUtility.h"
#include "DataChannel.h"

ScanRecordHash::ScanRecordHash(RecordHashGenerator *hashkey, unsigned int bucket_size)
	: m_table(bucket_size), m_hashkey(hashkey)
{
	m_number = 0;
    m_first = 0;
	for(int i = 0; i < m_table.size(); i ++)
		INIT_LIST_HEAD(&m_table[i]);

	pthread_mutex_init(&m_mutex, NULL);
	return;
}

ScanRecordHash::~ScanRecordHash()
{
	Clear();
	pthread_mutex_destroy(&m_mutex);
}

int ScanRecordHash::AddRecord(struct list_head *rec)
{
	unsigned int key = m_hashkey->GetHash(rec, m_table.size());
	struct list_head *pos;

	if(key >= m_table.size())
	{
		LogUtility::Log(LOG_LEVEL_ERROR, "ScanRecordHash::AddRecord key invalid %u\n", key);
		return -1;
	}

	pthread_mutex_lock(&m_mutex);
	struct list_head *head = &m_table[key];
	list_for_each(pos, head)
	{
		if(m_hashkey->IsKeySame(pos, rec))
		{
			struct list_head *chosen = m_hashkey->Choose(pos, rec);
			if (chosen != pos)
			{
				DeleteRecord(pos);
				list_add(rec, head);
			}
			else if (chosen != rec)
				m_hashkey->Free(rec);
			else
			{
				LogUtility::Log(LOG_LEVEL_ERROR, "ScanRecordHash::AddRecord chosen invalid\n");
				m_hashkey->Free(rec);
			}
			break;
		}
	}

	/* First entry for this key */
	if(pos == head)
	{
		list_add(rec, head);
		m_number ++;
        if(m_first== 0)
            DATA_INSTANCE->FirstSendData();
        m_first = 1;
	}

	pthread_mutex_unlock(&m_mutex);
	return 0;
}

void ScanRecordHash::Clear()
{
	pthread_mutex_lock(&m_mutex);
	for(int i = 0; i < m_table.size(); i ++)
	{
		struct list_head *head = &m_table[i];
		struct list_head *pos, *n;
		list_for_each_safe(pos, n, head)
		{
			DeleteRecord(pos);
		}
		INIT_LIST_HEAD(head);
	}
	m_number = 0;
	pthread_mutex_unlock(&m_mutex);
}

void ScanRecordHash::DeleteRecord(struct list_head *rec)
{
	list_del(rec);
	m_hashkey->Free(rec);
}

int ScanRecordHash::DumpRecord(int clean)
{
	LogUtility::Log(LOG_LEVEL_DEBUG, "ScanRecordHash DumpRecord number=%d",m_number);
	pthread_mutex_lock(&m_mutex);
	if((m_number <= 0) || (m_hashkey->PrepareBuffer(m_number) < 0))
	{
		pthread_mutex_unlock(&m_mutex);
		return 0;
	}
    int ret = m_number;
	for(int i = 0; i < m_table.size(); i ++)
	{
		struct list_head *head = &m_table[i];
		struct list_head *pos, *n;
		list_for_each_safe(pos, n, head)
		{
			m_hashkey->DumpRecord(pos);
			if(clean)
				DeleteRecord(pos);
		}
		INIT_LIST_HEAD(head);
	}
	m_number = 0;
	pthread_mutex_unlock(&m_mutex);
    return ret;
}
