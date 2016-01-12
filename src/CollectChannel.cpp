#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "LogUtility.h"
#include "CollectChannel.h"
#include "MacRecord.h"

extern "C"
{
#include "pcap.h"
#include "airodump-ng.h"
}

CollectChannel *CollectChannel::m_instance = NULL;

CollectChannel *CollectChannel::Instance()
{
    if(!m_instance)
    {
        m_instance = new CollectChannel();
    }

    return m_instance;
}

void CollectChannel::Destroy()
{
    if(m_instance != NULL)
    {
        delete m_instance;
    }
}

CollectChannel::CollectChannel()
{
    m_iface = getenv("WIRELESS_NETWORK_IFACE");
	m_exit = 0;
    
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init (&m_cond, NULL);
}

CollectChannel::~CollectChannel()
{
    Stop();
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}

void CollectChannel::Start()
{
    if(m_exit)
        m_exit = false;
    else
        pthread_create(&m_tid, NULL, this->ThreadLoop, this);

    return;
}

void CollectChannel::Stop()
{
    if(m_exit != true)
    {
    	pthread_mutex_lock(&m_mutex);
        m_exit = true;
        pthread_cond_signal(&m_cond);
        pthread_mutex_unlock(&m_mutex);
        airodum_exit(m_exit);
        pthread_join(m_tid, NULL);
    }

    return;
}

static void SignalHandler(int sig)
{
    LogUtility::Log(LOG_LEVEL_INFO, "SignalHandler catch signal 0x%x.", sig);
}

void callback_update_record(void *data)
{   
    MacRecord *sdata = (MacRecord *) data;	
    MAC_RECORD_TABLE->UpdateMacRecord(sdata);
}

void *CollectChannel::ThreadLoop(void *arg)
{    
	CollectChannel *collectChannel = reinterpret_cast<CollectChannel *> (arg);
    
	LogUtility::Log(LOG_LEVEL_DEBUG, "CollectChannel starting...");

    signal(SIGUSR1,SignalHandler);
    signal(SIGPIPE,SignalHandler);

	MacRecord rec;	
    airodum_scan_main(collectChannel->m_iface.c_str(), callback_update_record, &rec);

	pthread_exit(NULL);
}

