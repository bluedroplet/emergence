#define _GNU_SOURCE
#define _REENTRANT

#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "../../common/types.h"
#include "../../common/llist.h"
#include "rdtsc.h"

#include "../console.h"


struct timer_listener_t
{
	int pipe[2];
	struct timer_listener_t *next;
		
} *timer_listener0 = NULL;


pthread_t timer_thread_id;
//pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;

int timer_pipe[2];
int timer_alarmed, timer_exit;
uint64_t start_count, counts_per_second;
int timer_pid;


int create_timer_listener()
{
//	pthread_mutex_lock(&timer_mutex);
	
	struct timer_listener_t timer_listener;
	pipe(timer_listener.pipe);
	fcntl(timer_listener.pipe[0], F_SETFL, O_NONBLOCK);
	LL_ADD(struct timer_listener_t, &timer_listener0, &timer_listener);

//	pthread_mutex_unlock(&timer_mutex);

	return timer_listener.pipe[0];
}


void destroy_timer_listener(int read_fd)
{
//	pthread_mutex_lock(&timer_mutex);
	
	struct timer_listener_t *ctimer_listener = timer_listener0;
		
	while(ctimer_listener)
	{
		if(ctimer_listener->pipe[0] == read_fd)
		{
			LL_REMOVE(struct timer_listener_t, &timer_listener0, ctimer_listener);
			return;
		}
		
		ctimer_listener = ctimer_listener->next;
	}

//	pthread_mutex_unlock(&timer_mutex);
}


void handle_sigalrm(int i)
{
	timer_alarmed = 1;
}


void handle_sigusr1(int i)
{
	timer_exit = 1;
}


void *timer_thread(void *a)
{
	int pid = getpid();
	write(timer_pipe[1], &pid, 4);
	close(timer_pipe[1]);
	
	
	struct sigaction setup_action;
		
	// sigalrm
	
	setup_action.sa_handler = handle_sigalrm;
	sigemptyset(&setup_action.sa_mask);
	setup_action.sa_flags = SA_RESTART;
	if(sigaction(SIGALRM, &setup_action, NULL))
		printf("Couldn't setup sigalrm handler\n");
	
	struct itimerval tv;
	tv.it_interval.tv_usec = 10000;
	tv.it_interval.tv_sec = 0;
	tv.it_value.tv_usec = 10000;
	tv.it_value.tv_sec = 0;

	if(setitimer(ITIMER_REAL, &tv, NULL))
		printf("Couldn't setup interrupt timer\n");
	
	// sigusr1
	
	setup_action.sa_handler = handle_sigusr1;
	sigemptyset(&setup_action.sa_mask);
	setup_action.sa_flags = SA_RESTART;
	if(sigaction(SIGUSR1, &setup_action, NULL))
		printf("Couldn't setup sigalrm handler\n");
	
	sigset_t sigmask;
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGALRM);
	sigaddset(&sigmask, SIGUSR1);
	
	sigset_t oldmask;
	
	sigprocmask(SIG_BLOCK, &sigmask, &oldmask);
	
	while(1)
	{
		while(!timer_alarmed && !timer_exit)
			sigsuspend(&oldmask);
		
		if(timer_exit)
			pthread_exit(NULL);
		
	//	pthread_mutex_lock(&timer_mutex);
	
		struct timer_listener_t *ctimer_listener = timer_listener0;
			
		while(ctimer_listener)
		{
			char c;
			write(ctimer_listener->pipe[1], &c, 1);
		
			ctimer_listener = ctimer_listener->next;
		};
		
	//	pthread_mutex_unlock(&timer_mutex);
		
		timer_alarmed = 0;
	}
}


uint32_t get_tick()
{
	uint64_t count = rdtsc();
	
	return (uint32_t)(((count - start_count) * 200) / counts_per_second);
}


double get_double_time()
{
	uint64_t count = rdtsc();
	
	return (double)count / (double)counts_per_second;
}


void init_timer()
{
	double mhz;
	FILE *file = popen("grep \"cpu MHz\" /proc/cpuinfo", "r");
	fscanf(file, "%*s%*s%*s%lf", &mhz);
	pclose(file);
	
	counts_per_second = (uint64_t)(mhz * 1000000.0);
	
	console_print("Timer resolution: %u Hz\n", (uint32_t)counts_per_second);
	
	start_count = rdtsc();

	pipe(timer_pipe);
	pthread_create(&timer_thread_id, NULL, timer_thread, NULL);
	read(timer_pipe[0], &timer_pid, 4);
	close(timer_pipe[0]);
}


void kill_timer()
{
	kill(timer_pid, SIGUSR1);
	pthread_join(timer_thread_id, NULL);
}


void reset_start_count()
{
	start_count = rdtsc();
}
