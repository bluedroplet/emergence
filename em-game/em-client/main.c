#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/cvar.h"
#include "../shared/config.h"
#include "../shared/user.h"
#include "../shared/timer.h"
#include "render.h"
#include "network.h"
#include "console.h"
#include "control.h"
#include "input.h"
#include "game.h"
#include "rcon.h"
#include "tick.h"
#include "map.h"
#include "ping.h"
#include "sound.h"
#include "main.h"

#ifdef LINUX
#include "entry.h"
#endif

#ifdef WIN32
#include "win32/entry.h"
#endif


void create_cvars()
{
	create_cvar_string("name", "", 0);
	create_cvar_command("exec", exec_config_file);
}


void client_shutdown()
{
	console_print("Shutting down...\n");
	
	kill_sound();
	kill_game();
	kill_network();
	kill_render();
//	kill_input();
	dump_console();
	kill_console();
	write_config_file();

	terminate_process();
}


void client_shutdown_char(char *c)
{
	client_shutdown();
}


void client_error(const char *fmt, ...)
{
	char *msg;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&msg, fmt, ap);
	va_end(ap);

	console_print("Error: %s\n", msg);
	
	free(msg);
	
	client_shutdown();
}


void client_libc_error(const char *fmt, ...)
{
	char *msg;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&msg, fmt, ap);
	va_end(ap);

	console_print("Error: %s; %s.\n", msg, strerror(errno));
	
	free(msg);
	
	client_shutdown();
}


void process_messages(struct buffer_t *msg_buf)
{
	int stop = 0;
	int render = 0;

	struct buffer_t *stream;
	uint32_t index;
	uint64_t stamp;
	struct string_t *string;
	char c;
	int key, state;
	double ping;
	
	while(!stop)
	{
		switch(buffer_read_int(msg_buf))
		{
		case MSG_CONNECTION:
			game_process_connection();
			break;

		case MSG_DISCONNECTION:
			game_process_disconnection();
			break;

		case MSG_CONNLOST:
			game_process_conn_lost();
			break;

		case MSG_STREAM_TIMED:
			index = buffer_read_uint32(msg_buf);
			stamp = buffer_read_uint64(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			game_process_stream_timed(index, &stamp, stream);
			free_buffer(stream);
			break;
		
		case MSG_STREAM_UNTIMED:
			index = buffer_read_uint32(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			game_process_stream_untimed(index, stream);
			free_buffer(stream);
			break;
		
		case MSG_STREAM_TIMED_OOO:
			index = buffer_read_uint32(msg_buf);
			stamp = buffer_read_uint64(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			game_process_stream_timed_ooo(index, &stamp, stream);
			free_buffer(stream);
			break;
		
		case MSG_STREAM_UNTIMED_OOO:
			index = buffer_read_uint32(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			game_process_stream_untimed_ooo(index, stream);
			free_buffer(stream);
			break;
		
		case MSG_KEYPRESS:
			c = buffer_read_char(msg_buf);
			console_keypress(c);
			break;
		
		case MSG_UIFUNC:
			key = buffer_read_int(msg_buf);
			state = buffer_read_int(msg_buf);
			uifunc(key, state);
			break;

		case MSG_TEXT:
			string = (struct string_t*)buffer_read_uint32(msg_buf);
			console_print(string->text);
			free_string(string);
			break;

		case MSG_PING:
			ping = buffer_read_double(msg_buf);
		//	add_latency(ping);
			break;
		
		case MSG_RENDER:
			render = 1;
			break;

		case BUF_EOB:
			stop = 1;
			break;
		}
	}
	
	if(render)
		render_frame();
}


void init()
{
	console_print("Emergence Client v0.4\n");
	
	init_network();
	init_timer();

	init_user();

	create_cvars();
	init_console_cvars();
	init_render_cvars();
	init_map_cvars();
	create_input_cvars();
	init_tick_cvars();



	init_console();
	
	init_control();
	init_input();
	
	struct string_t *string = new_string_string(emergence_home_dir);
	string_cat_text(string, "/client.config");
	
	exec_config_file(string->text);
	
	free_string(string);


	init_render();
	init_rcon();
	init_ping();

	create_cvar_command("quit", client_shutdown_char);
	

	init_sound();
	init_game();
	
	
	render_frame();
	
/*	string = new_string_text("%s%s", home_dir->text, "/autoexec");
	
	exec_config_file(string->text);
	
	free_string(string);
*/
}
