#define _GNU_SOURCE
#define _REENTRANT

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "../common/types.h"
#include "../common/llist.h"
#include "../shared/cvar.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "shared/timer.h"
#include "shared/sgame.h"
#include "shared/network.h"
#include "shared/parse.h"
#include "network.h"
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
	
	struct conn_state_t *next;
		
} *conn_state0 = NULL;


#define CONN_STATE_VIRGIN	0
#define CONN_STATE_ACTIVE	1


void server_shutdown()
{
	console_print("Shutting down...\n");

	kill_network();
	kill_game();

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


void process_connection(uint32_t conn)
{
	net_emit_uint8(conn, EMNETMSG_PROTO_VER);
	net_emit_uint8(conn, EMNET_PROTO_VER);
	net_emit_end_of_stream(conn);

	struct conn_state_t conn_state = 
		{conn, get_double_time(), CONN_STATE_VIRGIN};
	LL_ADD(struct conn_state_t, &conn_state0, &conn_state);
}


void process_disconnection(uint32_t conn)
{
	struct conn_state_t *cstate = find_conn_state(conn);
	assert(cstate);
	
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
	case EMNETMSG_JOIN:
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


void process_msg_buf(struct buffer_t *msg_buf)
{
	int stop = 0;

	uint32_t conn;
	struct buffer_t *stream;
	uint32_t index;
	uint64_t stamp;
	struct string_t *string;

	while(!stop)
	{
		switch(buffer_read_int(msg_buf))
		{
		case MSG_CONNECTION:
			conn = buffer_read_uint32(msg_buf);
			process_connection(conn);
			break;

		case MSG_DISCONNECTION:
			conn = buffer_read_uint32(msg_buf);
			process_disconnection(conn);
			break;

		case MSG_CONNLOST:
			conn = buffer_read_uint32(msg_buf);
			process_conn_lost(conn);
			break;
		
		case MSG_STREAM_TIMED:
			index = buffer_read_uint32(msg_buf);
			stamp = buffer_read_uint64(msg_buf);
			conn = buffer_read_uint32(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			process_stream_timed(conn, index, &stamp, stream);
			free_buffer(stream);
			break;

		case MSG_STREAM_UNTIMED:
			index = buffer_read_uint32(msg_buf);
			conn = buffer_read_uint32(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			process_stream_untimed(conn, index, stream);
			free_buffer(stream);
			break;

		case MSG_STREAM_TIMED_OOO:
			index = buffer_read_uint32(msg_buf);
			stamp = buffer_read_uint64(msg_buf);
			conn = buffer_read_uint32(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			process_stream_timed_ooo(conn, index, &stamp, stream);
			free_buffer(stream);
			break;

		case MSG_STREAM_UNTIMED_OOO:
			index = buffer_read_uint32(msg_buf);
			conn = buffer_read_uint32(msg_buf);
			stream = (struct buffer_t*)buffer_read_uint32(msg_buf);
			process_stream_untimed_ooo(conn, index, stream);
			free_buffer(stream);
			break;
			
		case MSG_UPDATE_GAME:
			update_game();
			break;

		case MSG_COMMAND:
			string = (struct string_t*)buffer_read_uint32(msg_buf);
			process_command(string);
			free_string(string);
			break;

		case BUF_EOB:
			stop = 1;
			break;
		}
	}
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


void init()
{
	init_network();
	init_timer();
	init_game();

	create_cvar_command("daemonize", cf_go_daemon);
	create_cvar_command("quit", cf_quit);
	
	init_network_sig();
}
