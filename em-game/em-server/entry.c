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
#include "shared/timer.h"
#include "shared/cvar.h"
#include "main.h"
#include "game.h"
#include "console.h"

int as_daemon = 0;


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


const char *argp_program_version = "em-server " VERSION;
const char *argp_program_bug_address = "<jbrown@emergence.uk.net>";

static char doc[] = "Emergence Server";

static struct argp_option options[] = {
	{"daemon",	'd',	0,	0, "don't run in terminal (broken)"},
	{"port",	'p',	"PORT",	0, "port to listen on"},
	{ 0 }
};


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
	case 'd':
		as_daemon = 1;
		break;

	case 'p':
		net_set_listen_port(atoi(arg));
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, 0, doc };


int main(int argc, char *argv[])
{
	argp_parse(&argp, argc, argv, 0, 0, 0);

	init();
	
	if(as_daemon)
		go_daemon();
	
	main_thread();
	
	return 0;
}
