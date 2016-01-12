#ifndef _COLLECT_CHANNEL_H_
#define _COLLECT_CHANNEL_H_

#include <string>
#include <pthread.h>
#include "GeneralTimer.h"

using namespace std;

#define COLLECT_INSTANCE CollectChannel::Instance()

class CollectChannel
{
public:
	~CollectChannel();
	static CollectChannel *Instance();
	static void Destroy();
	void Start();
	void Stop();

protected:
	CollectChannel();

private:
    string m_iface;
    
	static void *ThreadLoop(void *arg);

    bool m_exit;
    pthread_t m_tid;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;


	static CollectChannel *m_instance;
};

#endif
