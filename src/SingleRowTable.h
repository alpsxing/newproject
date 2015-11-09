#ifndef _SINGLE_ROW_TABLE_H_
#define _SINGLE_ROW_TABLE_H_

#include <pthread.h>
#include "LogUtility.h"
#include "ConcreteTableBase.h"

using namespace std;

class SingleRowTableEventHandlerInterface
{
public:
    virtual int FieldChangeEventHandler(vector<int> changedItems) = 0;
};

class SingleRowTable : public ConcreteTableBase
{
public:
    int Refresh();
    int Commit();
    const SQLiteValuePair &GetValues() const
    {
        return m_values;
    }
    
    void SetHandler(SingleRowTableEventHandlerInterface *handler)
    {
    	CommonThreadLock thread_lock(&m_eventMutex);
    	m_eventHandler = handler;
    }

    int GetIntValue(int colid, int &value);
    int GetStringValue(int colid, string &value);
    int GetFloatValue(int colid, float &value);
    int GetSequenceValue(int colid, SQLiteSequence &value);

    int SetIntValue(int colid, int value);
    int SetStringValue(int colid, const string &value);
    int SetFloatValue(int colid, float value);
    int SetSequenceValue(int colid, SQLiteSequence &value);

protected:
    SingleRowTable(string dbName, string tableName, int maxrownum, int colnum, const SQLiteColumn *col) : ConcreteTableBase(dbName, tableName, maxrownum, colnum, col, true)
    {
        LogUtility::Log(LOG_LEVEL_ERROR, "SingleRowTable::SingleRowTable colnum %d.", m_colnum);
        for(int i = 0; i < m_colnum; i ++)
        {
            m_dirty.push_back(false);
        }
        m_eventHandler = NULL;
        pthread_mutex_init(&m_eventMutex, NULL);
        Refresh();
    }

    ~SingleRowTable()
    {
    	pthread_mutex_destroy(&m_eventMutex);
    }

    void SendNotification();
private:
    SQLiteValuePair m_values;
    vector<bool> m_dirty;
    SingleRowTableEventHandlerInterface *m_eventHandler;
    pthread_mutex_t m_eventMutex;
};

ostream& operator <<(ostream& os, const SingleRowTable& table);


#endif
