#define _GNU_SOURCE
#define _REENTRANT

/*
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>


#include "../../common/llist.h"
*/
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#include <sys/time.h>

#include "../../common/types.h"
#include "../../common/llist.h"
#include "rdtsc.h"

#include "../console.h"





struct alarm_listener_t
{
	int pipe[2];
	struct alarm_listener_t *next;
		
} *alarm_listener0 = NULL;


pthread_t alarm_thread_id;
//pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;


int create_alarm_listener()
{
//	pthread_mutex_lock(&alarm_mutex);
	
	struct alarm_listener_t alarm_listener;
	pipe(alarm_listener.pipe);
	fcntl(alarm_listener.pipe[0], F_SETFL, O_NONBLOCK);
	LL_ADD(struct alarm_listener_t, &alarm_listener0, &alarm_listener);

//	pthread_mutex_unlock(&alarm_mutex);

	return alarm_listener.pipe[0];
}


void destroy_alarm_listener(int read_fd)
{
//	pthread_mutex_lock(&alarm_mutex);
	
	struct alarm_listener_t *calarm_listener = alarm_listener0;
		
	while(calarm_listener)
	{
		if(calarm_listener->pipe[0] == read_fd)
		{
			LL_REMOVE(struct alarm_listener_t, &alarm_listener0, calarm_listener);
			return;
		}
		
		calarm_listener = calarm_listener->next;
	}

//	pthread_mutex_unlock(&alarm_mutex);
}


void *alarm_thread(void *a)
{
	struct itimerval tv;
	tv.it_interval.tv_usec = 10000;
	tv.it_interval.tv_sec = 0;
	tv.it_value.tv_usec = 10000;
	tv.it_value.tv_sec = 0;

	if(setitimer(ITIMER_REAL, &tv, NULL))
		printf("Couldn't setup interrupt alarm\n");
	
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGALRM);
	sigaddset(&sigmask, SIGUSR1);
	
	
	while(1)
	{
		int i;
		sigwait(&sigmask, &i);
		
		if(i == SIGUSR1)
			pthread_exit(NULL);

		
	//	pthread_mutex_lock(&alarm_mutex);
	
		struct alarm_listener_t *calarm_listener = alarm_listener0;
			
		while(calarm_listener)
		{
			char c;
			write(calarm_listener->pipe[1], &c, 1);
		
			calarm_listener = calarm_listener->next;
		};
		
	//	pthread_mutex_unlock(&alarm_mutex);
	}
}


void init_alarm()
{
	pthread_create(&alarm_thread_id, NULL, alarm_thread, NULL);
}


void kill_alarm()
{
	pthread_kill(alarm_thread_id, SIGUSR1);
	pthread_join(alarm_thread_id, NULL);
}
