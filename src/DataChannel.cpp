#include <vector>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <boost/algorithm/string.hpp>
#include "boost/format.hpp"
#include "DataChannel.h"
#include "LogUtility.h"
#include "RunningConfigTable.h"
#include "MacRecord.h"

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

	m_socket = -1;

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
        pthread_kill(m_tid, SIGUSR1);
        pthread_join(m_tid, NULL);
    }

    return;
}

void DataChannel::UpdateTxTimer(int invl)
{
	if(m_txTimer)
		DestroyTimer(m_txTimer);
	m_txTimer = CreateTimer(TX_TIMER_NAME, TIMER_SEC_TO_MSEC(invl), this, DataChannelTimerHandler, false);
	StartTimer(m_txTimer);
}

static void SignalHandler(int sig)
{
    LogUtility::Log(LOG_LEVEL_INFO, "SignalHandler catch signal 0x%x.", sig);
}

void *DataChannel::ThreadLoop(void *arg)
{
	DataChannel *dataChannel = reinterpret_cast<DataChannel *> (arg);

	dataChannel->UpdateRunningParas();

	LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel starting...");

    signal(SIGUSR1,SignalHandler);
    signal(SIGPIPE,SignalHandler);

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
			dataChannel->UpdateTxTimer(dataChannel->m_invl);
			pthread_mutex_unlock(&dataChannel->m_mutex);
			if(dataChannel->Connect() < 0)
			{
				LogUtility::Log(LOG_LEVEL_WARN, "DataChannel connect failed");
				dataChannel->UpdateTxTimer(10);
				continue;
			}
			if(dataChannel->Send()< 0)
			{
				LogUtility::Log(LOG_LEVEL_WARN, "DataChannel send failed");
				dataChannel->UpdateTxTimer(10);
			}
			dataChannel->Close();
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
		UpdateTxTimer(m_invl);
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

int DataChannel::Connect()
{
    struct sockaddr_in to;
    struct timeval timeout;
    fd_set fds;
    int ret;
    int flags;
    int error;

    LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Connect entry, server %s:%d.", m_addr.c_str(), m_port);

    m_socket = socket(AF_INET,SOCK_STREAM,0);
    if(m_socket < 0)
    {
        LogUtility::Log(LOG_LEVEL_ERROR, "DataChannel::Connect create socket failed.");
        return -1;
    }

    flags=fcntl(m_socket,F_GETFL,0);
    fcntl(m_socket,F_SETFL,flags|O_NONBLOCK);

    to.sin_family=AF_INET;
    to.sin_addr.s_addr=inet_addr(m_addr.c_str());
    to.sin_port = htons(m_port);

    ret = connect(m_socket, (struct sockaddr *)&to, sizeof(to));
    if(ret < 0)
    {
        if(errno != EINPROGRESS)
        {
            LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::Connect connect server failed.");
            Close();
            return -1;
        }
        else
        {
            FD_ZERO(&fds);
            FD_SET(m_socket,&fds);

            timeout.tv_sec = 10;
            timeout.tv_usec = 0;

            switch (select(m_socket + 1, NULL, &fds, NULL, &timeout))
            {
                case -1:
                    if(errno == EINTR)
                    {
                        LogUtility::Log(LOG_LEVEL_INFO, "DataChannel::Connect got a signal.");
                        Close();
                        return 1;
                    }
                    else
                    {
                        LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::Connect select failed.");
                        Close();
                        return -1;
                    }
                case 0:
                    LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::Connect not ok.");
                    Close();
                    return -1;
                default:
                    error = 0;
                    socklen_t len = sizeof(int);
                    if (( 0 == getsockopt(m_socket,SOL_SOCKET,SO_ERROR,&error,&len) ))
                    {
                        if( 0 == error )
                        {
                            LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Connect ok.");
                        }
                        else
                        {
                            LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::getsockopt get error.");
                            Close();
                            return -1;
                        }
                    }
                    else
                    {
                        LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::getsockopt failed.");
                        Close();
                        return -1;
                    }
                    break;
            }
        }
    }
    else
    {
        LogUtility::Log(LOG_LEVEL_WARN, "DataChannel::Connect already.");
    }

    LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Connect exit.");

    return 0;
}

void DataChannel::Close()
{
    if(m_socket >= 0)
    {
        close(m_socket);
        m_socket = -1;
    }
}

int DataChannel::Send()
{
    int totalSent = 0;
    unsigned char *buf;
	int len;
	int ret;

    LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Send.");

    buf = MAC_RECORD_TABLE->GetRecord(len);
    if(!buf || len <= 0)
    {
    	LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Send get record failed.");
    	return -1;
    }

    do{
	    ret = send(m_socket, buf+totalSent, len-totalSent, 0);
	    if(ret < 0)
	    {
		    if((errno == EAGAIN) || (errno == EWOULDBLOCK))
		    {
		    	LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Send channel blocked.");
			    return 0;
		    }
		    else
		    {
		    	LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Send failed.");
			    return -1;
		    }
	    }
	    else if(ret == 0)
	    {
	    	LogUtility::Log(LOG_LEVEL_DEBUG, "DataChannel::Send nothing.");
		    return 0;
	    }
	    else
	    {
		    totalSent += ret;
	    }
    }while(totalSent < len);

    return totalSent;
}
