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
#include <unistd.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <sys/poll.h>

#include "../common/prefix.h"
#include "../common/resource.h"

#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../common/user.h"
#include "shared/cvar.h"
#include "shared/config.h"
#include "shared/timer.h"
#include "shared/network.h"
#include "shared/parse.h"
#include "render.h"
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
#include "x.h"
#include "skin.h"
#include "key.h"

#ifdef LINUX
#include "entry.h"
#endif

#ifdef WIN32
#include "win32/entry.h"
#endif


void create_cvars()
{
	create_cvar_string("version", "", 0);
	create_cvar_string("name", "noname", 0);
	create_cvar_command("exec", (void*)exec_config_file);
}


void client_shutdown()
{
	console_print("Shutting down...\n");
	
	kill_key();
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


void process_network()
{
	uint32_t conn;
	struct buffer_t *stream;
	uint32_t index;
	uint64_t stamp;

	while(1)
	{
		uint32_t m;
		if(read(net_out_pipe[0], &m, 4) == -1)
			break;
		
		fcntl(net_out_pipe[0], F_SETFL, 0);
		
		switch(m)
		{
		case NETMSG_CONNECTING:
			game_process_connecting();
			break;
		
		case NETMSG_COOKIE_ECHOED:
			game_process_cookie_echoed();
			break;
		
		case NETMSG_CONNECTION:
			read(net_out_pipe[0], &conn, 4);
			game_process_connection(conn);
			break;

		case NETMSG_CONNECTION_FAILED:
			game_process_connection_failed(conn);
			break;
		
		case NETMSG_DISCONNECTION:
			read(net_out_pipe[0], &conn, 4);
			game_process_disconnection(conn);
			break;

		case NETMSG_CONNLOST:
			read(net_out_pipe[0], &conn, 4);
			game_process_conn_lost(conn);
			break;
		
		case NETMSG_STREAM_TIMED:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &stamp, 8);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			game_process_stream_timed(conn, index, &stamp, stream);
			free_buffer(stream);
			break;

		case NETMSG_STREAM_UNTIMED:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			game_process_stream_untimed(conn, index, stream);
			free_buffer(stream);
			break;

		case NETMSG_STREAM_TIMED_OOO:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &stamp, 8);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			game_process_stream_timed_ooo(conn, index, &stamp, stream);
			free_buffer(stream);
			break;

		case NETMSG_STREAM_UNTIMED_OOO:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			game_process_stream_untimed_ooo(conn, index, stream);
			free_buffer(stream);
			break;
		}

		fcntl(net_out_pipe[0], F_SETFL, O_NONBLOCK);
	}
}


void init()
{
	console_print("Emergence Client " VERSION "\n");
	
	
	init_timer();
	init_network();

	init_user();
	init_key();

	create_cvars();
	init_console_cvars();
	init_render_cvars();
	init_map_cvars();
	create_control_cvars();
//	create_input_cvars();
	init_tick_cvars();

	init_console();
	create_colour_cvars();
	
	struct string_t *string = new_string_string(emergence_home_dir);
	string_cat_text(string, "/client.config");
	
	if(!exec_config_file(string->text))
	{
		exec_config_file(find_resource("default-controls.config"));
	}
	else
	{
		char *ver = get_cvar_string("version");
		
		if(*ver == '\0')
		{
			struct string_t *command = new_string_text("rm ");
			string_cat_string(command, emergence_home_dir);
			string_cat_text(command, "/skins/default.skin*");
			
			console_print("%s\n", command->text);
			system(command->text);
			
			vid_mode = -1;	// find a nice mode

			exec_config_file(find_resource("default-controls.config"));
		}
		
		free(ver);
	}
	
	free_string(string);
	
	
	set_cvar_string("version", VERSION);
	
	init_skin();
	init_input();
	init_control();
	

	init_render();
	init_rcon();
	init_ping();

	create_cvar_command("quit", client_shutdown_char);
	

	init_sound();
	init_game();
	
	
	render_frame();
	
	string = new_string_text("%s%s", emergence_home_dir->text, "/client.autoexec");
	if(!exec_config_file(string->text))
		exec_config_file(find_resource("default-client.autoexec"));
	free_string(string);
}


void process_x_render_pipe()
{
	char c;
	while(read(x_render_pipe[0], &c, 1) != -1);
		
	render_frame();
}


void process_console_pipe()
{
	struct string_t *s = new_string();
	char c;
	while(read(console_pipe[0], &c, 1) != -1)
	{
	//	fcntl(console_pipe[0], F_SETFL, 0);
		
		if(c == 0)
		{
			parse_command(s->text);
			string_clear(s);
		}
		else
			string_cat_char(s, c);
		
	//	fcntl(console_pipe[0], F_SETFL, O_NONBLOCK);
	}
	
	free_string(s);
}


void main_thread()
{
	struct pollfd *fds;
	int fdcount;
	
	fdcount = 4;
	
	fds = calloc(sizeof(struct pollfd), fdcount);
	
	fds[0].fd = x_render_pipe[0]; fds[0].events |= POLLIN;
	fds[1].fd = net_out_pipe[0]; fds[1].events |= POLLIN;
	fds[2].fd = console_pipe[0]; fds[2].events |= POLLIN;
	fds[3].fd = key_out_pipe[0]; fds[3].events |= POLLIN;
	

	while(1)
	{
		if(poll(fds, fdcount, -1) == -1)
			return;

		if(fds[0].revents & POLLIN)
			process_x_render_pipe();
		
		if(fds[1].revents & POLLIN)
			process_network();
		
		if(fds[2].revents & POLLIN)
			process_console_pipe();
		
		if(fds[3].revents & POLLIN)
			process_key_out_pipe();
	}
}


/*
void main_thread()
{
	int epoll_fd = epoll_create(3);
	
	struct epoll_event ev;
	
	ev.events = EPOLLIN | EPOLLET;
	ev.data.u32 = 0;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, x_render_pipe[0], &ev);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.u32 = 1;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, net_out_pipe[0], &ev);

	ev.events = EPOLLIN | EPOLLET;
	ev.data.u32 = 2;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, console_pipe[0], &ev);

	while(1)
	{
		epoll_wait(epoll_fd, &ev, 1, -1);
		
		switch(ev.data.u32)
		{
		case 0:
			process_x_render_pipe();
			break;
		
		case 1:
			process_network();
			break;
		
		case 2:
			process_console_pipe();
			break;
		}
	}
}
*/
