#define _GNU_SOURCE
#define _REENTRANT

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <linux/joystick.h>

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
	struct js_event event;
	
	read(input_fd, &event, sizeof(struct js_event));
		
	switch(event.type & ~JS_EVENT_INIT)
	{
	case JS_EVENT_BUTTON:
		process_button(event.number, event.value);
		break;
	
	case JS_EVENT_AXIS:
		process_axis(event.number, (float)event.value);
		break;
	}
}


void create_input_cvars()
{
	create_cvar_string("input_dev", "/dev/js0", 0);
}


void init_input()
{
	return;
	
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
