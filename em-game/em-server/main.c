#define _GNU_SOURCE
#define _REENTRANT

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <sys/poll.h>

#include "../common/types.h"
#include "../common/llist.h"
#include "shared/cvar.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "shared/timer.h"
#include "shared/sgame.h"
#include "shared/network.h"
#include "shared/parse.h"
#include "../common/user.h"
#include "game.h"
#include "ping.h"
#include "console.h"
#include "main.h"
#include "entry.h"


struct conn_state_t
{
	uint32_t conn;
	double birth;
	int state;
	int type;
	
	struct conn_state_t *next;
		
} *conn_state0 = NULL;


#define CONN_STATE_VIRGIN	0
#define CONN_STATE_ACTIVE	1

int game_timer_fd;


void server_shutdown()
{
	console_print("Shutting down...\n");

	kill_network();
	kill_game();
	kill_timer();

	terminate_process();
}


void server_error(const char *fmt, ...)
{
	char *msg;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&msg, fmt, ap);
	va_end(ap);

	console_print("Error: %s.\n", msg);
	
	free(msg);
	
	server_shutdown();
}


void server_libc_error(const char *fmt, ...)
{
	char *msg;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&msg, fmt, ap);
	va_end(ap);

	console_print("Error: %s; %s.\n", msg, strerror(errno));
	
	free(msg);
	
	server_shutdown();
}


struct conn_state_t *find_conn_state(uint32_t conn)
{
	struct conn_state_t *ccstate = conn_state0;
		
	while(ccstate)
	{
		if(ccstate->conn == conn)
			return ccstate;
		
		ccstate = ccstate->next;
	}
	
	return NULL;
}


void process_connection(uint32_t conn, int type)
{
	net_emit_uint8(conn, EMMSG_PROTO_VER);
	net_emit_uint8(conn, EM_PROTO_VER);
	net_emit_end_of_stream(conn);

	switch(type)
	{
	case CONN_TYPE_LOCAL:
		printf("New local connection.\n");
		break;
	
	case CONN_TYPE_PRIVATE:
		printf("New private network connection.\n");
		break;
	
	case CONN_TYPE_PUBLIC:
	#ifdef NONAUTHENTICATING
		printf("New internet connection.\n");
		break;
	#else
		net_emit_uint8(conn, EMNETMSG_PRINT);
		net_emit_string(conn, "This server is not enabled for Internet play.\n");
		net_emit_end_of_stream(conn);
		em_disconnect(conn);
		return;
	#endif
	}

	struct conn_state_t conn_state = 
		{conn, get_wall_time(), CONN_STATE_VIRGIN, type};
	LL_ADD(struct conn_state_t, &conn_state0, &conn_state);
}


void process_disconnection(uint32_t conn)
{
	struct conn_state_t *cstate = find_conn_state(conn);
		
	if(!cstate)
		return;
	
	if(cstate->state == CONN_STATE_ACTIVE)
		game_process_disconnection(conn);		
	
	LL_REMOVE(struct conn_state_t, &conn_state0, cstate);
}


void process_conn_lost(uint32_t conn)
{
	struct conn_state_t *cstate = find_conn_state(conn);
	assert(cstate);
	
	if(cstate->state == CONN_STATE_ACTIVE)
		game_process_conn_lost(conn);		
	
	LL_REMOVE(struct conn_state_t, &conn_state0, cstate);
}


void process_virgin_stream(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	struct conn_state_t *cstate = find_conn_state(conn);
	assert(cstate);
	
	switch(buffer_read_uint8(stream))
	{
	case EMMSG_JOIN:
		game_process_join(conn, index, stream);
		cstate->state = CONN_STATE_ACTIVE;
		break;
	}
}


void process_stream_timed(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	struct conn_state_t *cstate = find_conn_state(conn);
	assert(cstate);
	
	switch(cstate->state)
	{
	case CONN_STATE_VIRGIN:
		process_virgin_stream(conn, index, stream);
		break;
	
	case CONN_STATE_ACTIVE:
		game_process_stream_timed(conn, index, stamp, stream);
		break;
	}
}


void process_stream_untimed(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	struct conn_state_t *cstate = find_conn_state(conn);
	assert(cstate);
	
	switch(cstate->state)
	{
	case CONN_STATE_VIRGIN:
		process_virgin_stream(conn, index, stream);
		break;
	
	case CONN_STATE_ACTIVE:
		game_process_stream_untimed(conn, index, stream);
		break;
	}
}


void process_stream_timed_ooo(uint32_t conn, uint32_t index, uint64_t *stamp, struct buffer_t *stream)
{
	struct conn_state_t *cstate = find_conn_state(conn);
	assert(cstate);
	
	switch(cstate->state)
	{
	case CONN_STATE_VIRGIN:
		process_virgin_stream(conn, index, stream);
		break;
	
	case CONN_STATE_ACTIVE:
		game_process_stream_timed_ooo(conn, index, stamp, stream);
		break;
	}
}


void process_stream_untimed_ooo(uint32_t conn, uint32_t index, struct buffer_t *stream)
{
	struct conn_state_t *cstate = find_conn_state(conn);
	assert(cstate);
	
	switch(cstate->state)
	{
	case CONN_STATE_VIRGIN:
		process_virgin_stream(conn, index, stream);
		break;
	
	case CONN_STATE_ACTIVE:
		game_process_stream_untimed_ooo(conn, index, stream);
		break;
	}
}


void process_command(struct string_t *string)
{
	parse_command(string->text);
}


void cf_go_daemon(char *c)
{
	if(!as_daemon)
	{
		go_daemon();
		as_daemon = 1;
	}
	else
		console_print("Process already daemonized!\n");
}


void cf_quit(char *c)
{
	server_shutdown();
}


void process_console()
{
	char c;
	
	while(1)
	{
		if(read(STDIN_FILENO, &c, 1) == -1)
			break;
	
		fcntl(STDIN_FILENO, F_SETFL, 0);
		
		struct string_t *string = new_string();
		
		while(c != '\n')
		{
			string_cat_char(string, c);
			read(STDIN_FILENO, &c, 1);
		}
		
		process_command(string);
		free_string(string);
		
		fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	}
}


void process_network()
{
	uint32_t conn;
	struct buffer_t *stream;
	uint32_t index;
	uint64_t stamp;
	int type;

	while(1)
	{
		uint32_t m;
		if(read(net_out_pipe[0], &m, 4) == -1)
			break;
		
		fcntl(net_out_pipe[0], F_SETFL, 0);
		
		switch(m)
		{
		case NETMSG_CONNECTION:
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &type, 4);
			process_connection(conn, type);
			break;

		case NETMSG_DISCONNECTION:
			read(net_out_pipe[0], &conn, 4);
			process_disconnection(conn);
			break;

		case NETMSG_CONNLOST:
			read(net_out_pipe[0], &conn, 4);
			process_conn_lost(conn);
			break;
		
		case NETMSG_STREAM_TIMED:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &stamp, 8);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			process_stream_timed(conn, index, &stamp, stream);
			free_buffer(stream);
			break;

		case NETMSG_STREAM_UNTIMED:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			process_stream_untimed(conn, index, stream);
			free_buffer(stream);
			break;

		case NETMSG_STREAM_TIMED_OOO:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &stamp, 8);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			process_stream_timed_ooo(conn, index, &stamp, stream);
			free_buffer(stream);
			break;

		case NETMSG_STREAM_UNTIMED_OOO:
			read(net_out_pipe[0], &index, 4);
			read(net_out_pipe[0], &conn, 4);
			read(net_out_pipe[0], &stream, 4);
			process_stream_untimed_ooo(conn, index, stream);
			free_buffer(stream);
			break;
		}

		fcntl(net_out_pipe[0], F_SETFL, O_NONBLOCK);
	}
}


void process_game_timer()
{
	uint32_t m;
	while(read(game_timer_fd, &m, 1) != -1);
		
	update_game();
}


void main_thread()
{
	struct pollfd *fds;
	int fdcount;
	
	fdcount = 3;
	
	fds = calloc(sizeof(struct pollfd), fdcount);
	
	fds[0].fd = STDIN_FILENO; fds[0].events |= POLLIN;
	fds[1].fd = net_out_pipe[0]; fds[1].events |= POLLIN;
	fds[2].fd = game_timer_fd; fds[2].events |= POLLIN;
	

	while(1)
	{
		if(poll(fds, fdcount, -1) == -1)
			return NULL;

		if(fds[0].revents & POLLIN)
			process_console();
		
		if(fds[1].revents & POLLIN)
			process_network();
		
		if(fds[2].revents & POLLIN)
			process_game_timer();
	}
}


/*
void main_thread()
{
	int epoll_fd = epoll_create(3);
	
	struct epoll_event ev = 
	{
		.events = EPOLLIN | EPOLLET
	};

	ev.data.u32 = 0;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

	ev.data.u32 = 1;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, net_out_pipe[0], &ev);

	ev.data.u32 = 2;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, game_timer_fd, &ev);

	while(1)
	{
		epoll_wait(epoll_fd, &ev, 1, -1);
		
		switch(ev.data.u32)
		{
		case 0:
			process_console();
			break;
		
		case 1:
			process_network();
			break;
		
		case 2:
			process_game_timer();
			break;
		}
	}
}
*/


void init()
{
	console_print("Emergence Server " VERSION "\n");
	
	init_user();
	init_network();
	init_game();
	init_timer();
	
	game_timer_fd = create_timer_listener();

	create_cvar_command("daemonize", cf_go_daemon);
	create_cvar_command("quit", cf_quit);
}
