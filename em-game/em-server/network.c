#define _GNU_SOURCE
#define _REENTRANT

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <arpa/inet.h>

#include "../common/types.h"
#include "../common/llist.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/cvar.h"
#include "../shared/rdtsc.h"
#include "../shared/timer.h"
#include "../shared/network.h"
#include "network.h"
#include "game.h"
#include "main.h"
#include "console.h"
#include "entry.h"


struct in_packet_ll_t
{
	struct packet_t packet;
	int payload_size;
	int stream_delivered;		// set on the first packet of a stream once that stream has been delivered ooo
		
	struct in_packet_ll_t *next;
};


struct out_packet_ll_t
{
	struct packet_t packet;
	double next_resend_time;
	int size;
		
	struct out_packet_ll_t *next;
};


struct conn_cookie_t
{
	double expire_time;
	uint32_t cookie;
	uint32_t ip;
	uint16_t port;
	struct conn_cookie_t *next;
		
} *conn_cookie0 = NULL;


struct conn_t
{
	int state;
	int begun;

	struct sockaddr_in sockaddr;	// address of client

	uint32_t next_read_index;		// index of the next packet to be read
	uint32_t next_process_index;	// index of the next packet to be processed
	uint32_t next_write_index;		// index of the next packet to be sent
	
	struct in_packet_ll_t *in_packet0;		// unprocessed received packets in index order
	struct out_packet_ll_t *out_packet0;	// unacknowledged sent packets in index order
	
	double last_time;	// the time that the last ack was received or 
						// first packet in out_packet was sent if it was empty or
						// the time the connection was disconnected
	
	struct packet_t cpacket;	// the packet currently being constructed
	int cpacket_payload_size;
	
	struct conn_t *next;

} *conn0 = NULL;


#define NETSTATE_CONNECTED		0	// the server has acknowledged the connection
#define NETSTATE_DISCONNECTING	1	// the server has requested a disconnection
#define NETSTATE_DISCONNECTED	2	// the client has requested a disconnection

#define OUT_OF_ORDER_RECV_AHEAD		10
#define CONNECTION_COOKIE_TIMEOUT	4.0
#define CONNECTION_TIMEOUT			5.0
#define CONNECTION_MAX_OUT_PACKETS	400
#define STREAM_RESEND_DELAY			0.1


int udp_socket = -1;


struct conn_t *new_conn()
{
	struct conn_t *next = conn0;
	
	conn0 = calloc(1, sizeof(struct conn_t));
	conn0->next = next;

	return conn0;
}


void delete_conn(struct conn_t *conn)
{
	LL_REMOVE_ALL(struct in_packet_ll_t, &conn->in_packet0);
	LL_REMOVE_ALL(struct out_packet_ll_t, &conn->out_packet0);
	LL_REMOVE(struct conn_t, &conn0, conn);
}


void delete_all_conns()
{
	while(conn0)
		delete_conn(conn0);
}


struct conn_t *get_conn(struct sockaddr_in *sockaddr)
{
	struct conn_t *conn = conn0;

	while(conn)
	{
		if(conn->sockaddr.sin_addr.s_addr == sockaddr->sin_addr.s_addr &&
			conn->sockaddr.sin_port == sockaddr->sin_port)
			return conn;
		
		conn = conn->next;
	}

	return NULL;
}


int conn_exist(struct conn_t *conn)
{
	struct conn_t *cconn = conn0;

	while(cconn)
	{
		if(cconn == conn)
			return 1;
		
		cconn = cconn->next;
	}
	
	return 0;
}


int insert_in_packet(struct conn_t *conn, struct packet_t *packet, int size)
{
	struct in_packet_ll_t *temp;
	struct in_packet_ll_t *cpacket;
	uint32_t index;
	
	
	// if packet0 is NULL, then create new packet here

	if(!conn->in_packet0)
	{
		conn->in_packet0 = calloc(1, sizeof(struct in_packet_ll_t));
		memcpy(conn->in_packet0, packet, size);
		conn->in_packet0->payload_size = size - EMNETHEADER_SIZE;
		return 1;
	}

	
	index = conn->in_packet0->packet.header & EMNETINDEX_MASK;
	
	if(index == (packet->header & EMNETINDEX_MASK))
		return 0;		// the packet is already present in this list
	
	if(index < conn->next_process_index)
		index += EMNETINDEX_MAX + 1;
	
	if(index > (packet->header & EMNETINDEX_MASK))
	{
		temp = conn->in_packet0;
		conn->in_packet0 = calloc(1, sizeof(struct in_packet_ll_t));
		memcpy(conn->in_packet0, packet, size);
		conn->in_packet0->payload_size = size - EMNETHEADER_SIZE;
		conn->in_packet0->next = temp;
		return 1;
	}

	
	// search through the rest of the list
	// to find the packet before the first
	// packet to have an index greater
	// than index


	cpacket = conn->in_packet0;
	
	while(cpacket->next)
	{
		uint32_t index = cpacket->next->packet.header & EMNETINDEX_MASK;
		
		if(index == (packet->header & EMNETINDEX_MASK))
			return 0;		// the packet is already present in this list

		if(index < conn->next_process_index)
			index += EMNETINDEX_MAX + 1;
		
		if(index > (packet->header & EMNETINDEX_MASK))
		{
			temp = cpacket->next;
			cpacket->next = calloc(1, sizeof(struct in_packet_ll_t));
			cpacket = cpacket->next;
			memcpy(cpacket, packet, size);
			cpacket->payload_size = size - EMNETHEADER_SIZE;
			cpacket->next = temp;
			return 1;
		}

		cpacket = cpacket->next;
	}

	
	// we have reached the end of the list and not found
	// a packet that has an index greater than or equal to packet->index

	cpacket->next = calloc(1, sizeof(struct in_packet_ll_t));
	cpacket = cpacket->next;
	memcpy(cpacket, packet, size);
	cpacket->payload_size = size - EMNETHEADER_SIZE;
	return 1;
}


int output_cpacket(struct conn_t *conn)
{
	// count packets
	
	struct out_packet_ll_t **packet = &conn->out_packet0;
	int num_packets = 0;

	while(*packet)
	{
		++num_packets;
		packet = &(*packet)->next;
	}
	
	if(num_packets >= CONNECTION_MAX_OUT_PACKETS)
		return 0;


	// get time
		
	double time = get_double_time();
	
	if(num_packets == 0)
		conn->last_time = time;

	
	// send packet
	
	int size = conn->cpacket_payload_size + EMNETHEADER_SIZE;
	
	sendto(udp_socket, (char*)&conn->cpacket, size, 0, 
		&conn->sockaddr, sizeof(struct sockaddr_in));
			

	// create new packet at end of list
	
	switch(conn->cpacket.header & EMNETCLASS_MASK)
	{
	case EMNETCLASS_CONTROL:
		time += STREAM_RESEND_DELAY;
		break;
	
	case EMNETCLASS_STREAM:
		time += STREAM_RESEND_DELAY;
		conn->cpacket.header |= EMNETFLAG_STREAMRESNT;
		break;
	}

	*packet = calloc(1, sizeof(struct out_packet_ll_t));
	memcpy(*packet, &conn->cpacket, size);
	(*packet)->next_resend_time = time;
	(*packet)->size = size;
	return 1;
}


void send_conn_cookie(struct sockaddr_in *sockaddr)
{
	struct conn_cookie_t *cookie = malloc(sizeof(struct conn_cookie_t));
	cookie->expire_time = get_double_time() + CONNECTION_COOKIE_TIMEOUT;
	cookie->cookie = mrand48() & EMNETCOOKIE_MASK;
	cookie->ip = sockaddr->sin_addr.s_addr;
	cookie->port = sockaddr->sin_port;
	cookie->next = conn_cookie0;
	conn_cookie0 = cookie;
	
	uint32_t header = EMNETFLAG_COOKIE | cookie->cookie;
	sendto(udp_socket, (char*)&header, EMNETHEADER_SIZE, 0, sockaddr, sizeof(struct sockaddr_in));
}


int query_conn_cookie(uint32_t cookie, struct sockaddr_in *sockaddr)
{
	struct conn_cookie_t *ccookie = conn_cookie0;
		
	while(ccookie)
	{
		if(ccookie->cookie == cookie)
		{
			if(ccookie->ip == sockaddr->sin_addr.s_addr &&
				ccookie->port == sockaddr->sin_port)
			{
				LL_REMOVE(struct conn_cookie_t, &conn_cookie0, ccookie);	// optimize me away
				return 1;
			}
		}
		
		ccookie = ccookie->next;
	}
	
	return 0;
}


void send_connect(struct conn_t *conn)
{
	uint32_t header = EMNETFLAG_CONNECT | conn->next_read_index;
	sendto(udp_socket, (char*)&header, EMNETHEADER_SIZE, 0, (struct sockaddr*)&conn->sockaddr, 
		sizeof(struct sockaddr_in));
}


void send_ack(struct sockaddr_in *addr, uint32_t index)
{
	uint32_t header = EMNETFLAG_ACKWLDGEMNT | index;
	sendto(udp_socket, (char*)&header, EMNETHEADER_SIZE, 0, addr, sizeof(struct sockaddr_in));
}


void process_ack(struct conn_t *conn, uint32_t index)
{
	struct out_packet_ll_t *temp;

	if(!conn->out_packet0)
		return;

	if((conn->out_packet0->packet.header & EMNETINDEX_MASK) == index)	// the packet is the first on the list
	{
		temp = conn->out_packet0->next;
		free(conn->out_packet0);
		conn->out_packet0 = temp;
	}
	else
	{
		// search the rest of the list

		struct out_packet_ll_t *packet = conn->out_packet0;

		while(packet->next)
		{
			if((packet->next->packet.header & EMNETINDEX_MASK) == index)
			{
				temp = packet->next->next;
				free(packet->next);
				packet->next = temp;
				break;
			}

			packet = packet->next;
		}
	}


	if(conn->state == NETSTATE_DISCONNECTING && !conn->out_packet0)
	{
		delete_conn(conn);
		return;
	}
	
	// record the time for timeout purposes
	
	conn->last_time = get_double_time();
}


void emit_process_connection(struct conn_t *conn)
{
	buffer_cat_uint32(msg_buf, (uint32_t)MSG_CONNECTION);
	buffer_cat_uint32(msg_buf, (uint32_t)conn);
}


void emit_process_disconnection(struct conn_t *conn)
{
	buffer_cat_uint32(msg_buf, (uint32_t)MSG_DISCONNECTION);
	buffer_cat_uint32(msg_buf, (uint32_t)conn);
}


void emit_process_conn_lost(struct conn_t *conn)
{
	buffer_cat_uint32(msg_buf, (uint32_t)MSG_CONNLOST);
	buffer_cat_uint32(msg_buf, (uint32_t)conn);
}


void process_in_order_stream(struct conn_t *conn, struct in_packet_ll_t *end)
{
	struct buffer_t *buf = new_buffer();
	struct in_packet_ll_t *temp;
	int untimed = 0;
	
	if(conn->in_packet0->stream_delivered)
		untimed = 1;
	
	uint32_t index = conn->in_packet0->packet.header & EMNETINDEX_MASK;
	
	while(conn->in_packet0 != end)
	{
		if(conn->in_packet0->packet.header & EMNETFLAG_STREAMRESNT)
			untimed = 1;
			
		buffer_cat_buf(buf, (char*)&conn->in_packet0->packet.payload, conn->in_packet0->payload_size);

		temp = conn->in_packet0;
		conn->in_packet0 = conn->in_packet0->next;
		free(temp);
	}

	if(conn->in_packet0->packet.header & EMNETFLAG_STREAMRESNT)
		untimed = 1;
	
	buffer_cat_buf(buf, (char*)&conn->in_packet0->packet.payload, conn->in_packet0->payload_size);

	temp = conn->in_packet0;
	conn->in_packet0 = conn->in_packet0->next;
	free(temp);

	if(untimed)
	{
		buffer_cat_uint32(msg_buf, (uint32_t)MSG_STREAM_UNTIMED);
		buffer_cat_uint32(msg_buf, index);
	}
	else
	{
		buffer_cat_uint32(msg_buf, (uint32_t)MSG_STREAM_TIMED);
		buffer_cat_uint32(msg_buf, index);
		buffer_cat_uint64(msg_buf, rdtsc());
	}
	
	buffer_cat_uint32(msg_buf, (uint32_t)conn);
	buffer_cat_uint32(msg_buf, (uint32_t)buf);
}


void process_out_of_order_stream(struct conn_t *conn, struct in_packet_ll_t *start, struct in_packet_ll_t *end)
{
	if(start->stream_delivered)
		return;
	
	struct in_packet_ll_t *cpacket = start;
	struct buffer_t *buf = new_buffer();
	int untimed = 0;
	
	while(cpacket != end)
	{
		if(cpacket->packet.header & EMNETFLAG_STREAMRESNT)
			untimed = 1;
			
		buffer_cat_buf(buf, (char*)&cpacket->packet.payload, cpacket->payload_size);
	}

	if(cpacket->packet.header & EMNETFLAG_STREAMRESNT)
		untimed = 1;
	
	buffer_cat_buf(buf, (char*)&cpacket->packet.payload, cpacket->payload_size);

	if(untimed)
	{
		buffer_cat_uint32(msg_buf, (uint32_t)MSG_STREAM_UNTIMED_OOO);
		buffer_cat_uint32(msg_buf, start->packet.header & EMNETINDEX_MASK);
	}
	else
	{
		buffer_cat_uint32(msg_buf, (uint32_t)MSG_STREAM_TIMED_OOO);
		buffer_cat_uint32(msg_buf, start->packet.header & EMNETINDEX_MASK);
		buffer_cat_uint64(msg_buf, rdtsc());
	}
	
	buffer_cat_uint32(msg_buf, (uint32_t)conn);
	buffer_cat_uint32(msg_buf, (uint32_t)buf);
	
	start->stream_delivered = 1;
}


void check_stream(struct conn_t *conn)
{
	struct in_packet_ll_t *cpacket = conn->in_packet0;
	uint32_t nindex = conn->next_process_index;

	check_first:
	
	if((cpacket->packet.header & EMNETINDEX_MASK) == nindex)
	{
		if((cpacket->packet.header & (EMNETCLASS_MASK | EMNETFLAG_MASK)) == EMNETFLAG_DISCONNECT)
		{
			conn->state = NETSTATE_DISCONNECTED;
			emit_process_disconnection(conn);
			return;
		}
		
		
		// packet is definately stream first
		
		while(1)
		{
			if((cpacket->packet.header & EMNETINDEX_MASK) == conn->next_read_index)
				conn->next_read_index = (conn->next_read_index + 1) % (EMNETINDEX_MAX + 1);
	
			if(cpacket->packet.header & EMNETFLAG_STREAMLAST)
			{
				process_in_order_stream(conn, cpacket);
				
				cpacket = conn->in_packet0;
				conn->next_process_index = ++nindex;
				
				if(!cpacket)
					return;
				
				goto check_first;		
			}

			cpacket = cpacket->next;

			if(!cpacket)
				return;
			
			nindex++;
			
			if((cpacket->packet.header & EMNETINDEX_MASK) != nindex)
				break;
		}
	}
	
	
	// deliver out-of-order streams
	
	struct in_packet_ll_t *start_packet = NULL;
	
	while(cpacket)
	{
		if((cpacket->packet.header & (EMNETCLASS_MASK | EMNETFLAG_MASK)) == EMNETFLAG_DISCONNECT)
			return;
		
		// packet is definately stream
		
		if(!start_packet)
		{
			check_first_ooo:

			if(cpacket->packet.header & EMNETFLAG_STREAMFIRST)
			{
				if(cpacket->packet.header & EMNETFLAG_STREAMLAST)
				{
					process_out_of_order_stream(conn, cpacket, cpacket);
				}
				else
				{
					start_packet = cpacket;
					nindex = (start_packet->packet.header & EMNETINDEX_MASK) + 1;
				}
			}
		}
		else
		{
			if((cpacket->packet.header & EMNETINDEX_MASK) != nindex)
			{
				start_packet = NULL;
				goto check_first_ooo;
			}
			
			if(cpacket->packet.header & EMNETFLAG_STREAMLAST)
			{
				process_out_of_order_stream(conn, start_packet, cpacket);
				start_packet = NULL;
			}
			else
			{
				nindex++;
			}
		}
		
		cpacket = cpacket->next;
	}
}


void udp_data()
{
	struct packet_t packet;
	struct sockaddr_in recv_addr;
	

	// receive packet and address
	
	size_t addr_size = sizeof(struct sockaddr_in);
	int size = recvfrom(udp_socket, (char*)&packet, EMNETPACKET_SIZE, 0, (struct sockaddr*)&recv_addr, &addr_size);
		
	
	// check packet has a header
	
	if(size < EMNETHEADER_SIZE)
		return;
	
	uint32_t index = packet.header & EMNETINDEX_MASK;
	

	// see if the packet is from an address where a connection is established
	
	struct conn_t *conn = get_conn(&recv_addr);
	
	
	// process packet
	
	switch(packet.header & EMNETCLASS_MASK)
	{
	case EMNETCLASS_CONTROL:
		
		switch(packet.header & EMNETFLAG_MASK)
		{
		case EMNETFLAG_CONNECT:
			
			if(size != EMNETHEADER_SIZE)
				break;
		
			if(index != EMNETCONNECT_MAGIC)
				break;
			
			if(conn)
			{
				if(!conn->begun)
				{
					conn->last_time = get_double_time();
					send_connect(conn);
				}
				else
				{
					send_conn_cookie(&recv_addr);
				}
			}
			else
			{
				send_conn_cookie(&recv_addr);
			}
			
			break;
		
			
		case EMNETFLAG_COOKIE:
			
			if(size != EMNETHEADER_SIZE)
				break;
		
			if(!query_conn_cookie(index, &recv_addr))
				break;
				
			if(conn)
			{
				if(conn->state == NETSTATE_CONNECTED)
					emit_process_disconnection(conn);
				
				delete_conn(conn);
			}
			
			conn = new_conn();
			conn->state = NETSTATE_CONNECTED;
			memcpy(&conn->sockaddr, &recv_addr, sizeof(struct sockaddr_in));
			conn->next_process_index = conn->next_read_index = 
				conn->next_write_index = index;
			send_connect(conn);
			conn->cpacket.header = conn->next_write_index++;
			conn->next_write_index %= EMNETINDEX_MAX + 1;
			conn->cpacket.header |= EMNETCLASS_STREAM | EMNETFLAG_STREAMFIRST;
			emit_process_connection(conn);
			break;
		
			
		case EMNETFLAG_DISCONNECT:
			
			// ensure packet is from a client
			
			if(!conn)
				break;
			
			if(size != EMNETHEADER_SIZE)
				break;
		
			send_ack(&recv_addr, index);
			
			conn->begun = 1;
			
			if(conn->state == NETSTATE_DISCONNECTING)
			{
				conn->last_time = get_double_time();
				conn->state = NETSTATE_DISCONNECTED;
				break;
			}
			
			if(conn->state != NETSTATE_CONNECTED)
				break;
		
			if(index < conn->next_read_index)
				index += EMNETINDEX_MAX + 1;
			
			if(index > conn->next_read_index + OUT_OF_ORDER_RECV_AHEAD)
				break;
			
			if(insert_in_packet(conn, &packet, size))
				check_stream(conn); // see if this packet has completed a stream
	
			break;
		
			
		case EMNETFLAG_ACKWLDGEMNT:
			
			if(size != EMNETHEADER_SIZE)
				break;
		
			if(!conn)
				break;
			
			process_ack(conn, index);
			
			conn->begun = 1;
			
			break;
		}
		
		break;

		
	case EMNETCLASS_STREAM:
		
		// ensure packet is from a client
		
		if(!conn)
			break;
		
		if(size == EMNETHEADER_SIZE)
			break;
		
		send_ack(&recv_addr, index);
		
		if(conn->state != NETSTATE_CONNECTED)
			break;
		
		conn->begun = 1;
			
		if(index < conn->next_read_index)
			index += EMNETINDEX_MAX + 1;
		
		if(index > conn->next_read_index + OUT_OF_ORDER_RECV_AHEAD)
			break;
		
		if(insert_in_packet(conn, &packet, size))
			check_stream(conn); // see if this packet has completed a stream
		
		break;
		
		
	case EMNETCLASS_MISC:
		
		switch(packet.header & EMNETFLAG_MASK)
		{
		case EMNETFLAG_PING:
			
			if(size != EMNETHEADER_SIZE)
				break;
		
			packet.header = EMNETCLASS_MISC | EMNETFLAG_PONG | (packet.header & EMNETCOOKIE_MASK);
			sendto(udp_socket, (char*)&packet, EMNETHEADER_SIZE, 0, 
				&recv_addr, sizeof(struct sockaddr_in));
					
			break;
		
				
		case EMNETFLAG_PONG:
			
			if(size != EMNETHEADER_SIZE)
				break;
			
			break;
		
			
		case EMNETFLAG_SERVERINFO:
			break;
		}
		
		break;
	}	
}


struct string_t *get_text_addr(void *conn)
{
	assert(conn);
	
	return new_string_text("%s:%u", inet_ntoa(((struct conn_t*)conn)->sockaddr.sin_addr), 
		((struct conn_t*)conn)->sockaddr.sin_port);
}


void network_alarm()
{
	double time = get_double_time();
	
	
	// remove old cookies
	
	struct conn_cookie_t *cookie = conn_cookie0;
		
	while(cookie)
	{
		if(time > cookie->expire_time)
		{
			struct conn_cookie_t *temp = cookie->next;
			LL_REMOVE(struct conn_cookie_t, &conn_cookie0, cookie);		// optimize me away
			cookie = temp;
			continue;
		}
		
		cookie = cookie->next;
	}
	
	
	struct conn_t *conn = conn0;

	while(conn)
	{
		struct out_packet_ll_t *packet = conn->out_packet0;
		
		switch(conn->state)
		{
		case NETSTATE_CONNECTED:
			
			// check if conn has timed out
			
			if(packet && time - conn->last_time > CONNECTION_TIMEOUT)
			{
				emit_process_conn_lost(conn);
				
				struct conn_t *temp = conn->next;
				delete_conn(conn);
				conn = temp;
				continue;
			}
			
			
			// resend unacknowledged packets
	
			while(packet)
			{
				if(time > packet->next_resend_time)
				{
					sendto(udp_socket, (char*)(packet), packet->size, 0, 
						(struct sockaddr*)&conn->sockaddr, sizeof(struct sockaddr_in));
						
					packet->next_resend_time += (floor((time - packet->next_resend_time) / 
						STREAM_RESEND_DELAY) + 1.0) * STREAM_RESEND_DELAY;
				}
	
				packet = packet->next;
			}
			
			break;
		
			
		case NETSTATE_DISCONNECTING:
			
			if(time - conn->last_time > CONNECTION_TIMEOUT)
			{
				struct conn_t *temp = conn->next;
				delete_conn(conn);
				conn = temp;
				continue;
			}
			
			// resend unacknowledged packets
	
			while(packet)
			{
				if(time > packet->next_resend_time)
				{
					sendto(udp_socket, (char*)(packet), packet->size, 0, 
						(struct sockaddr*)&conn->sockaddr, sizeof(struct sockaddr_in));
						
					packet->next_resend_time += (floor((time - packet->next_resend_time) / 
						STREAM_RESEND_DELAY) + 1.0) * STREAM_RESEND_DELAY;
				}
	
				packet = packet->next;
			}
			
			break;
		
			
		case NETSTATE_DISCONNECTED:
			// check if conn has timed out
			
			if(time - conn->last_time > CONNECTION_TIMEOUT)
			{
				struct conn_t *temp = conn->next;
				delete_conn(conn);
				conn = temp;
				continue;
			}
		
			break;
		}

		conn = conn->next;
	}
}


void send_cpacket(struct conn_t *conn)
{
	if(!output_cpacket(conn))
	{
		delete_conn(conn);
		emit_process_conn_lost(conn);
		return;
	}

	conn->cpacket.header = EMNETCLASS_STREAM | conn->next_write_index++;
	conn->next_write_index %= EMNETINDEX_MAX + 1;
}


void net_emit_uint32(uint32_t temp_conn, uint32_t val)
{
	struct conn_t *conn = (struct conn_t*)temp_conn;
	assert(conn);
	
	mask_sigs();
	
	// make sure conn was not deleted before sigs masked
	
	if(!conn_exist(conn))
		goto end;
	
	if(conn->state != NETSTATE_CONNECTED)
		goto end;
	
	switch(conn->cpacket_payload_size)
	{
	case EMNETPAYLOAD_SIZE:

		send_cpacket(conn);

		*(uint32_t*)&conn->cpacket.payload[0] = val;
		conn->cpacket_payload_size = 4;

		break;


	case EMNETPAYLOAD_SIZE - 1:

		conn->cpacket.payload[conn->cpacket_payload_size] = ((uint8_t*)&val)[0];
		conn->cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket(conn);

		*(uint16_t*)&conn->cpacket.payload[0] = *(uint16_t*)&((uint8_t*)&val)[1];
		conn->cpacket.payload[2] = ((uint8_t*)&val)[3];
		conn->cpacket_payload_size = 3;

		break;


	case EMNETPAYLOAD_SIZE - 2:

		*(uint16_t*)&conn->cpacket.payload[conn->cpacket_payload_size] = ((uint16_t*)(void*)&val)[0];
		conn->cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket(conn);

		*(uint16_t*)&conn->cpacket.payload[0] = ((uint16_t*)(void*)&val)[1];
		conn->cpacket_payload_size = 2;

		break;


	case EMNETPAYLOAD_SIZE - 3:

		*(uint16_t*)&conn->cpacket.payload[conn->cpacket_payload_size] = ((uint16_t*)(void*)&val)[0];
		conn->cpacket.payload[conn->cpacket_payload_size + 2] = ((uint8_t*)&val)[2];
		conn->cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket(conn);

		conn->cpacket.payload[0] = ((uint8_t*)&val)[3];
		conn->cpacket_payload_size = 1;

		break;


	default:

		*(uint32_t*)&conn->cpacket.payload[conn->cpacket_payload_size] = val;
		conn->cpacket_payload_size += 4;

		break;
	}
	
	end:
	unmask_sigs();
}


void net_emit_int(uint32_t temp_conn, int val)
{
	net_emit_uint32(temp_conn, *(uint32_t*)&val);
}


void net_emit_float(uint32_t temp_conn, float val)
{
	net_emit_uint32(temp_conn, *(uint32_t*)(void*)&val);
}


void net_emit_uint16(uint32_t temp_conn, uint16_t val)
{
	struct conn_t *conn = (struct conn_t*)temp_conn;
	assert(conn);
	
	mask_sigs();
	
	// make sure conn was not deleted before sigs masked
	
	if(!conn_exist(conn))
		goto end;
	
	if(conn->state != NETSTATE_CONNECTED)
		goto end;
	
	switch(conn->cpacket_payload_size)
	{
	case EMNETPAYLOAD_SIZE:

		send_cpacket(conn);

		*(uint16_t*)&conn->cpacket.payload[0] = val;
		conn->cpacket_payload_size = 2;

		break;


	case EMNETPAYLOAD_SIZE - 1:

		conn->cpacket.payload[conn->cpacket_payload_size] = ((uint8_t*)&val)[0];
		conn->cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket(conn);

		conn->cpacket.payload[0] = ((uint8_t*)&val)[1];
		conn->cpacket_payload_size = 1;

		break;


	default:

		*(uint16_t*)&conn->cpacket.payload[conn->cpacket_payload_size] = val;
		conn->cpacket_payload_size += 2;

		break;
	}
	
	end:
	unmask_sigs();
}


void net_emit_uint8(uint32_t temp_conn, uint8_t val)
{
	struct conn_t *conn = (struct conn_t*)temp_conn;
	assert(conn);
	
	mask_sigs();
	
	// make sure conn was not deleted before sigs masked
	
	if(!conn_exist(conn))
		goto end;
	
	if(conn->state != NETSTATE_CONNECTED)
		goto end;
	
	switch(conn->cpacket_payload_size)
	{
	case EMNETPAYLOAD_SIZE:
		send_cpacket(conn);
		conn->cpacket.payload[0] = val;
		conn->cpacket_payload_size = 1;
		break;

	default:
		conn->cpacket.payload[conn->cpacket_payload_size] = val;
		conn->cpacket_payload_size++;
		break;
	}
	
	end:
	unmask_sigs();
}


void net_emit_char(uint32_t temp_conn, char c)
{
	net_emit_uint8(temp_conn, *(uint8_t*)&c);
}


void net_emit_string(uint32_t temp_conn, char *cc)		// optomize me
{
	if(!cc)
		return;

	while(*cc)
		net_emit_char(temp_conn, *cc++);

	net_emit_char(temp_conn, '\0');
}


void net_emit_buf(uint32_t temp_conn, void *buf, int size)
{
	;
}


void net_emit_end_of_stream(uint32_t temp_conn)
{
	struct conn_t *conn = (struct conn_t*)temp_conn;
	assert(conn);

	mask_sigs();
	
	// make sure conn was not deleted before sigs masked
	
	if(!conn_exist(conn))
		goto end;
	
	if(conn->state != NETSTATE_CONNECTED)
		goto end;
	
	conn->cpacket.header |= EMNETFLAG_STREAMLAST;
	send_cpacket(conn);
	conn->cpacket.header |= EMNETFLAG_STREAMFIRST;
	conn->cpacket_payload_size = 0;
	
	end:
	unmask_sigs();
}


void disconnect(uint32_t temp_conn)		// server-side disconnection forced
{
/*	
	
	
	
	
	struct conn_t *conn = (struct conn_t*)temp_conn;
	assert(conn);

	mask_sigs();
	
	conn->begun = 1;
	
	// make sure conn was not deleted before sigs masked
	
	if(!conn_exist(conn))
		goto end;
	
	if(!conn->active)
		goto end;

	double time = get_double_time() + STREAM_RESEND_DELAY;
	
	
	// send disconnection packet to client
	
	struct packet_t packet = {
		EMNETFLAG_DISCONNECT | conn->next_write_index
	};
	
	if(!add_out_packet(conn, &packet, EMNETHEADER_SIZE, &time))
	{
		delete_conn(conn);
		emit_process_conn_lost(conn);
		goto end;
	}
	
	sendto(udp_socket, (char*)&packet, EMNETHEADER_SIZE, 0, (struct sockaddr*)&conn->sockaddr, 
		sizeof(struct sockaddr_in));
			

	// declare connection inactive (it will be deleted when client acknowledges)
	
	conn->active = 0;

end:
		
	unmask_sigs();
*/
}


void init_network()
{
	size_t size = 20;
	char *hostname = malloc(size);

	while(gethostname(hostname, size) == -1)
	{
		if(errno != ENAMETOOLONG)
		{
			free(hostname);
			server_libc_error("gethostname failure");
		}

		size *= 2;

		if(size > 65536)
		{
			free(hostname);
			server_error("Host name far too long!");
		}
			
		hostname = realloc(hostname, size);
	}

	console_print("Host: %s\n", hostname);

	struct hostent *hostent;
	hostent = gethostbyname(hostname);
	free(hostname);
	if(!hostent)
		server_libc_error("gethostbyname failure");

	struct in_addr **addr = (struct in_addr**)hostent->h_addr_list;

	struct string_t *addr_list = new_string_text(inet_ntoa(**addr));

	addr++;

	int multiple = 0;

	while(*addr)
	{
		string_cat_text(addr_list, "; ");
		string_cat_text(addr_list, inet_ntoa(**addr));

		addr++;
		multiple = 1;
	}

	create_cvar_string("ipaddr", addr_list->text, CVAR_PROTECTED);

	if(multiple)
		console_print("IP addresses: %s\n", addr_list->text);
	else
		console_print("IP address: %s\n", addr_list->text);
	
	free_string(addr_list);

	udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
	if(udp_socket < 0)
		server_libc_error("socket failure");

	struct sockaddr_in name;
	name.sin_family = AF_INET;
	name.sin_port = htons(EMNET_PORT);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(udp_socket, (struct sockaddr *) &name, sizeof (name)) < 0)
		server_libc_error("bind failure");

	
	// seed to rng
	
	uint64_t time = rdtsc();
	
	seed48((unsigned short int*)(void*)&time);
}


void init_network_sig()
{
	if(fcntl(udp_socket, F_SETOWN, getpid()) == -1)
		server_libc_error("F_SETOWN");
		
	if(fcntl(udp_socket, F_SETFL, O_ASYNC) == -1)
		server_libc_error("F_SETFL");
}

/*
void kill_network()
{
	if(net_state == NETSTATE_CONNECTED)
	{
		console_print("Disconnecting...\n");
		em_disconnect(NULL);
		
		// wait for disconnection
		
		sigset_t mask, oldmask;
		
		// Set up the mask of signals to temporarily block. 
		sigemptyset(&mask);
		sigaddset(&mask, SIGIO);
		sigaddset(&mask, SIGALRM);
		sigaddset(&mask, SIGUSR1);
		
		// Wait for a signal to arrive. 
		sigprocmask(SIG_BLOCK, &mask, &oldmask);
		
		while(net_state != NETSTATE_DEAD)
			sigsuspend(&oldmask);
		
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}
}
*/

void kill_network()		// FIXME
{
	delete_all_conns();
	close(udp_socket);
}
