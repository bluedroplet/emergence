#define _GNU_SOURCE
#define _REENTRANT

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <argp.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/timer.h"
#include "../shared/user.h"
#include "main.h"
#include "render.h"
#include "input.h"
#include "network.h"
#include "console.h"
#include "x.h"
#include "entry.h"
#include "control.h"

volatile int sigio_process = 0;
volatile int sigalrm_process = 0;

struct buffer_t *msg_buf = NULL;
	
struct timeval timeout = 
	{0, 0};
		
	
sigset_t sigmask;
	

void handle_sigio(int i)
{
	fd_set set;
	
	while(1)
	{
		if(!sigio_process)
			return;
		
		FD_ZERO(&set);
		
		if(sigio_process & SIGIO_PROCESS_NETWORK)
			FD_SET(udp_socket, &set);
		
		if(sigio_process & SIGIO_PROCESS_INPUT)
			FD_SET(input_fd, &set);
		
		if(sigio_process & SIGIO_PROCESS_X)
			FD_SET(x_fd, &set);
		
		select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		
		if(sigio_process & SIGIO_PROCESS_NETWORK)
		{
			if(FD_ISSET(udp_socket, &set))
			{
				process_udp_data();
				continue;
			}
		}
		
		if(sigio_process & SIGIO_PROCESS_INPUT)
		{
			if(FD_ISSET(input_fd, &set))
			{
				process_input();
				continue;
			}
		}
		
		if(sigio_process & SIGIO_PROCESS_X)
		{
			if(FD_ISSET(x_fd, &set))
			{
				process_x();
				continue;
			}
		}
		
		return;
	}
}


void handle_sigalrm(int i)
{
	if(sigalrm_process & SIGALRM_PROCESS_NETWORK)
		process_network_alarm();
	
	if(sigalrm_process & SIGALRM_PROCESS_CONTROL)
		process_control_alarm();
}

void handle_sigfpe(int i)
{
	printf("handle_sigfpe\n");
}



void init_signals()
{
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGIO);
	sigaddset(&sigmask, SIGALRM);
	struct sigaction setup_action;

	
	// sigfpe
	
	setup_action.sa_handler = handle_sigfpe;
	sigemptyset(&setup_action.sa_mask);
	setup_action.sa_flags = SA_RESTART;
	
	if(sigaction(SIGFPE, &setup_action, NULL) == -1)
		client_libc_error("Couldn't setup sigfpe handler");

	
	// sigio
	
	setup_action.sa_handler = handle_sigio;
	sigemptyset(&setup_action.sa_mask);
	sigaddset(&setup_action.sa_mask, SIGALRM);
	setup_action.sa_flags = SA_RESTART;
	
	if(sigaction(SIGIO, &setup_action, NULL) == -1)
		client_libc_error("Couldn't setup sigio handler");

	
	// sigalrm
	
	setup_action.sa_handler = handle_sigalrm;
	sigemptyset(&setup_action.sa_mask);
	sigaddset(&setup_action.sa_mask, SIGIO);
	setup_action.sa_flags = SA_RESTART;
	
	if(sigaction(SIGALRM, &setup_action, NULL) == -1)
		client_libc_error("Couldn't setup sigalrm handler");

	struct itimerval tv;
	tv.it_interval.tv_usec = 10000;
	tv.it_interval.tv_sec = 0;
	tv.it_value.tv_usec = 10000;
	tv.it_value.tv_sec = 0;

	if(setitimer(ITIMER_REAL, &tv, NULL) < 0)
		client_libc_error("Couldn't setup interrupt timer");
}


void mask_sigs()
{
	sigprocmask(SIG_BLOCK, &sigmask, NULL);
}


void unmask_sigs()
{
	sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
}


void terminate_process()
{
	exit(EXIT_SUCCESS);
}

const char *argp_program_version = "em-client 0.2";
const char *argp_program_bug_address = "<jbrown@emergence.uk.net>";

static char doc[] = "Emergence Client";

static struct argp_option options[] = {
	{ 0 }
};


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, 0 , doc };


int main(int argc, char *argv[])
{
	argp_parse(&argp, argc, argv, 0, 0, 0);

	msg_buf = new_buffer();
	
	init_signals();
	init();
	
	sigset_t oldmask, sigmask;
	
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGIO);
	sigaddset(&sigmask, SIGALRM);
	
	struct buffer_t *new_buf = new_buffer();
	
	while(1)
	{
		sigprocmask(SIG_BLOCK, &sigmask, &oldmask);
		
		while(!buffer_more(msg_buf))
			sigsuspend(&oldmask);
		
		buffer_cat_buffer(new_buf, msg_buf);
		buffer_clear(msg_buf);
		
		sigprocmask(SIG_UNBLOCK, &sigmask, NULL);
		
		process_messages(new_buf);
		buffer_clear(new_buf);
	}
}
