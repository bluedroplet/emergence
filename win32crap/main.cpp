#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#include "../../common/string.h"
#include "../../common/buffer.h"
#include "../../common/cvar.h"
#include "../../common/config.h"
#include "winmain.h"
#include "pipe.h"
#include "render.h"
#include "network.h"
#include "console.h"
#include "control.h"
#include "input.h"
#include "game.h"
#include "../rcon.h"


HANDLE hMainThread = NULL;
dwordlong clockfreq;

void create_cvars()
{
	create_cvar_string("name", "", 0);
	create_cvar_command("exec", exec_config_file);
}


void init_timer()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&clockfreq);

	string s("Timer resolution: ");
	s.cat(clockfreq);
	s.cat("Hz\n");

	console_print(s.text);
}


dwordlong get_time()
{
	dwordlong count;

	QueryPerformanceCounter((LARGE_INTEGER*)&count);

	return (dwordlong)((count * 500) / clockfreq);
}


void shutdown()
{
	SetThreadPriority(hMainThread, THREAD_PRIORITY_NORMAL);
	kill_game();
	kill_input();
	kill_network();
	kill_render();
	kill_thread_pipes();
	dump_console();
	kill_console();
	write_config_file();

	terminate_process();
}


void shutdown(int)
{
	shutdown();
}


void shutdown(char*)
{
	shutdown();
}


void main()
{
	hMainThread = GetCurrentThread();

	create_cvars();
	init_console_cvars();
	init_render_cvars();
	init_console();

	console_print("NetFighter v0.1\n");
	
	init_control();
	exec_config_file("nfcl.cfg");

	init_timer();

	init_thread_pipes();
	init_render();
	init_network();

	init_input();

	init_rcon();

	create_cvar_command("quit", shutdown);
	exec_config_file("autoexec.cfg");

	init_game();

	HANDLE events[3] = {from_render_pipe, from_net_pipe, hKeyboardEvent};

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	while(1)
	{
		int event = MsgWaitForMultipleObjects(3, events, 0, INFINITE, QS_ALLINPUT);
		
		event -= WAIT_OBJECT_0;

		switch(event)
		{
		case 0:		// Message from render pipe
			process_render_message();
			break;

		case 1:		// Message from network pipe
			process_network_message();
			break;
		
		case 2:
			keyboard_callback();
			break;

		case 3:
			process_wnd_msgs();
			break;
		}
	}
}


