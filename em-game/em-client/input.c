#define _GNU_SOURCE
#define _REENTRANT

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <linux/input.h>

#include "../common/types.h"
#include "../shared/cvar.h"
#include "../shared/timer.h"
#include "../common/stringbuf.h"
#include "console.h"
#include "main.h"
#include "control.h"
#include "entry.h"


int input_fd = -1;

void process_input()
{
	struct input_event event;
	
	read(input_fd, &event, sizeof(struct input_event));
		
	switch(event.type)
	{
	case EV_KEY:
		process_button(event.code - BTN_MOUSE, event.value);
		break;
	
	case EV_REL:
		process_axis(event.code, (float)*(int*)&event.value);
		break;
	}
}


void create_input_cvars()
{
	create_cvar_string("input_dev", "/dev/input/event2", 0);
}


void init_input()
{
	console_print("Opening input device: ");
	
	input_fd = open(get_cvar_string("input_dev"), O_RDONLY);
	if(input_fd < 0)
		goto error;

	console_print("ok\nGetting input device to generate signals: ");
	
	if(fcntl(input_fd, F_SETOWN, getpid()) == -1)
		goto error;
	
	if(fcntl(input_fd, F_SETFL, O_ASYNC) == -1)
		goto error;
	
	console_print("ok\n");
	
	sigio_process |= SIGIO_PROCESS_INPUT;
	
	return;
	
error:
	
	console_print("fail\n");
	perror(NULL);
	client_shutdown();
}


void kill_input()
{
}
