#define _GNU_SOURCE
#define _REENTRANT

#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <argp.h>

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/timer.h"
#include "../shared/user.h"
#include "../shared/cvar.h"
#include "main.h"
#include "network.h"
#include "game.h"
#include "console.h"

struct buffer_t *msg_buf = NULL;
int as_daemon = 0;
sigset_t sigmask;


void process_console()
{
	struct string_t *string = new_string();
	
	char c;
	
	read(STDIN_FILENO, &c, 1);
	
	while(c != '\n')
	{
		string_cat_char(string, c);
		read(STDIN_FILENO, &c, 1);
	}
	
	buffer_cat_uint32(msg_buf, (uint32_t)MSG_COMMAND);
	buffer_cat_uint32(msg_buf, (uint32_t)string);
}


void handle_sigio(int i)
{
	while(1)
	{
		fd_set set;
		struct timeval timeout;
		
		FD_ZERO(&set);
		FD_SET(udp_socket, &set);
		
		if(!as_daemon)
			FD_SET(STDIN_FILENO, &set);
		
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		
		select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		
		if(FD_ISSET(udp_socket, &set))
		{
			udp_data();
			continue;
		}

		if(!as_daemon)
		{
			if(FD_ISSET(STDIN_FILENO, &set))
			{
				process_console();
				continue;
			}
		}
		
		return;
	}
}


void handle_sigalrm(int i)
{
	network_alarm();

	buffer_cat_uint32(msg_buf, (uint32_t)MSG_UPDATE_GAME);
}


void init_signals()
{
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGIO);
	sigaddset(&sigmask, SIGALRM);
	
	struct sigaction setup_action;


	// sigio
	
	setup_action.sa_handler = handle_sigio;
	sigemptyset(&setup_action.sa_mask);
	sigaddset(&setup_action.sa_mask, SIGALRM);
	setup_action.sa_flags = SA_RESTART;
	if(sigaction(SIGIO, &setup_action, NULL))
		server_libc_error("Couldn't setup sigio handler");

	if(!as_daemon)
	{
		// make user input generate signals
		
		if(fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1)
			server_libc_error("F_SETOWN");
			
		if(fcntl(STDIN_FILENO, F_SETFL, O_ASYNC) == -1)
			server_libc_error("F_SETFL");
	}
	

	// sigalrm
	
	setup_action.sa_handler = handle_sigalrm;
	sigemptyset(&setup_action.sa_mask);
	sigaddset(&setup_action.sa_mask, SIGIO);
	setup_action.sa_flags = SA_RESTART;
	if(sigaction(SIGALRM, &setup_action, NULL))
		server_libc_error("Couldn't setup sigalrm handler");
	
	struct itimerval tv;
	tv.it_interval.tv_usec = 10000;
	tv.it_interval.tv_sec = 0;
	tv.it_value.tv_usec = 10000;
	tv.it_value.tv_sec = 0;

	if(setitimer(ITIMER_REAL, &tv, NULL))
		server_libc_error("Couldn't setup interrupt timer");
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


void go_daemon()
{
	console_print("Daemonizing...\n");
	
	pid_t pid = fork();

	if(pid != 0)
		exit(EXIT_SUCCESS);

	// Become session leader
	setsid();

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	
	stdout = fopen("nfsv.log", "w");
}


const char *argp_program_version = "em-server 0.2";
const char *argp_program_bug_address = "<jbrown@emergence.uk.net>";

static char doc[] = "Emergence Server";

static struct argp_option options[] = {
	{"daemon",	'd',	0,	0, "don't run in terminal"},
	{ 0 }
};


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
	case 'd':
		as_daemon = 1;
		break;

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
	mask_sigs();
	init_user();
	init();
	unmask_sigs();
	
	if(as_daemon)
		go_daemon();
	
	sigset_t oldmask;
	
	struct buffer_t *new_buf = new_buffer();

	while(1)
	{
		sigprocmask(SIG_BLOCK, &sigmask, &oldmask);
		
		while(!buffer_more(msg_buf))
			sigsuspend(&oldmask);
		
		buffer_cat_buffer(new_buf, msg_buf);
		buffer_clear(msg_buf);
		
		sigprocmask(SIG_UNBLOCK, &sigmask, NULL);

		process_msg_buf(new_buf);
		buffer_clear(new_buf);
	}

	return 0;
}
