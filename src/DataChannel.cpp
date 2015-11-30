#include <vector>
#include <boost/algorithm/string.hpp>
#include "boost/format.hpp"
#include "DataChannel.h"
#include "LogUtility.h"
#include "RunningConfigTable.h"

typedef std::vector<string> split_vector_type;
using boost::format;
using namespace boost;

DataChannel *DataChannel::m_instance = NULL;

static void DataChannelTimerHandler(std::string &name, void *data, void *data2)
{
	DataChannel *handler = (DataChannel *)data;
    if(handler != NULL)
    {
        handler->HandleTimer(name, data2);
    }
}

DataChannel *DataChannel::Instance()
{
    if(!m_instance)
    {
        m_instance = new DataChannel();
    }

    return m_instance;
}

void DataChannel::Destroy()
{
    if(m_instance != NULL)
    {
        delete m_instance;
    }
}

DataChannel::DataChannel()
{
	m_txTimer = NULL;
	m_invl = -1;
	m_should_update = 0;
	m_should_send = 0;
	m_exit = 0;

	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init (&m_cond, NULL);
}

DataChannel::~DataChannel()
{
	DestroyTimer(m_txTimer);
    Stop();
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

void DataChannel::Start()
{
    if(m_exit)
        m_exit = false;
    else
        pthread_create(&m_tid, NULL, this->ThreadLoop, this);

    return;
}

void DataChannel::Stop()
{
    if(m_exit != true)
    {
    	pthread_mutex_lock(&m_mutex);
        m_exit = true;
        pthread_cond_signal(&m_cond);
        pthread_mutex_unlock(&m_mutex);
        pthread_join(m_tid, NULL);
    }

    return;
}

void DataChannel::UpdateTxTimer()
{
	if(m_txTimer)
		DestroyTimer(m_txTimer);
	m_txTimer = CreateTimer(TX_TIMER_NAME, TIMER_SEC_TO_MSEC(m_invl), this, DataChannelTimerHandler, true);
	StartTimer(m_txTimer);
}

void *DataChannel::ThreadLoop(void *arg)
{
	DataChannel *dataChannel = reinterpret_cast<DataChannel *> (arg);

	dataChannel->UpdateRunningParas();

	LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel stChangeHelloInvl(invl, 1);arting...");

	while(1)
	{
		pthread_mutex_lock(&dataChannel->m_mutex);
		if(!dataChannel -> m_exit &&
		   !dataChannel->m_should_send &&
		   !dataChannel->m_should_update)
			pthread_cond_wait(&dataChannel->m_cond, &dataChannel->m_mutex);

		if(dataChannel->m_exit)
		{
			pthread_mutex_unlock(&dataChannel->m_mutex);
			break;
		}

		if(dataChannel->m_should_update)
		{
			dataChannel->m_should_update = 0;
			dataChannel->UpdateRunningParas();
		}

		if(dataChannel->m_should_send)
		{
			dataChannel->m_should_send = 0;
			pthread_mutex_unlock(&dataChannel->m_mutex);
		}
		else
			pthread_mutex_unlock(&dataChannel->m_mutex);
	}
	StopTimer(dataChannel->m_txTimer);
	pthread_exit(NULL);
}

void DataChannel::RunningTableUpdated()
{
	pthread_mutex_lock(&m_mutex);
	m_should_update = 1;
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutex);
}

void DataChannel::UpdateRunningParas()
{
	RunningConfigTable running_table;
	std::string dataurl;
	split_vector_type para_pair;
	int invl;

	running_table.GetDataUrl(dataurl);
	running_table.GetUpInvl(invl);

	if(invl < 0)
	{
		LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::UpdateRunningParas Invalid invl %d", invl);
		invl = 300;
	}

	if(m_invl != invl)
	{
		m_invl = invl;
		UpdateTxTimer();
	}

	trim(dataurl);

	split(para_pair, dataurl, is_any_of(":"), token_compress_on);
	if(para_pair.size() != 2)
	{
		LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::UpdateRunningParas Invalid url %s", dataurl.c_str());
		return;
	}

	m_port = stringToNum<int>(para_pair[1]);
	if(m_port < 0)
	{
		LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::UpdateRunningParas port invalid %d", m_port);
		return;
	}
	m_addr = para_pair[0];

	return;
}

void DataChannel::HandleTimer(std::string &name, void *data)
{
	if(name == TX_TIMER_NAME)
	{
	    if(!m_should_send)
	    {
	    	pthread_mutex_lock(&m_mutex);
	    	m_should_send = 1;
	        pthread_cond_signal(&m_cond);
	        pthread_mutex_unlock(&m_mutex);
	    }
	}
}
