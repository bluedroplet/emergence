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

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <arpa/inet.h>

#include "../common/types.h"
#include "../common/llist.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../shared/cvar.h"
#include "../shared/sgame.h"
#include "../shared/timer.h"
#include "../shared/rdtsc.h"
#include "../shared/network.h"
#include "network.h"
#include "main.h"
#include "game.h"
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


struct sockaddr_in server_addr;	// address of server
struct sockaddr_in next_addr;	// address of next server

uint32_t next_read_index;		// index of the next packet to be read
uint32_t next_process_index;	// index of the next packet to be processed
uint32_t next_write_index;		// index of the next packet to be sent

struct in_packet_ll_t *in_packet0;		// unprocessed received packets in index order
struct out_packet_ll_t *out_packet0;	// unacknowledged sent packets in index order

double last_net_time;	// the time that the last ack was received or first packet in out_packet was sent if it was empty

struct packet_t cpacket;	// the packet currently being constructed
int cpacket_payload_size;


int net_state = NETSTATE_DEAD;

int udp_socket = -1;

int net_in_use;
int net_fire_alarm;


#define OUT_OF_ORDER_RECV_AHEAD		10
#define CONNECTION_TIMEOUT			5.0
#define CONNECTION_MAX_OUT_PACKETS	400
#define CONNECT_RESEND_DELAY		0.5
#define STREAM_RESEND_DELAY			0.1


int insert_in_packet(struct packet_t *packet, int size)
{
	struct in_packet_ll_t *temp;
	struct in_packet_ll_t *cpacket;
	uint32_t index;
	
	
	// if packet0 is NULL, then create new packet here

	if(!in_packet0)
	{
		in_packet0 = calloc(1, sizeof(struct in_packet_ll_t));
		memcpy(in_packet0, packet, size);
		in_packet0->payload_size = size - EMNETHEADER_SIZE;
		return 1;
	}

	
	index = in_packet0->packet.header & EMNETINDEX_MASK;
	
	if(index == (packet->header & EMNETINDEX_MASK))
		return 0;		// the packet is already present in this list
	
	if(index < next_process_index)
		index += EMNETINDEX_MAX + 1;
	
	if(index > (packet->header & EMNETINDEX_MASK))
	{
		temp = in_packet0;
		in_packet0 = calloc(1, sizeof(struct in_packet_ll_t));
		memcpy(in_packet0, packet, size);
		in_packet0->payload_size = size - EMNETHEADER_SIZE;
		in_packet0->next = temp;
		return 1;
	}

	
	// search through the rest of the list
	// to find the packet before the first
	// packet to have an index greater
	// than index


	cpacket = in_packet0;
	
	while(cpacket->next)
	{
		uint32_t index = cpacket->next->packet.header & EMNETINDEX_MASK;
		
		if(index == (packet->header & EMNETINDEX_MASK))
			return 0;		// the packet is already present in this list

		if(index < next_process_index)
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


int output_cpacket()
{
	// count packets
	
	struct out_packet_ll_t **packet = &out_packet0;
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
		last_net_time = time;

	
	// send packet
	
	int size = cpacket_payload_size + EMNETHEADER_SIZE;
	
	sendto(udp_socket, (char*)&cpacket, size, 0, 
		&server_addr, sizeof(struct sockaddr_in));
			

	// create new packet at end of list
		
	switch(cpacket.header & EMNETCLASS_MASK)
	{
	case EMNETCLASS_CONTROL:
		switch(cpacket.header & EMNETFLAG_MASK)
		{
		case EMNETFLAG_CONNECT:
			time += CONNECT_RESEND_DELAY;
			break;
		
		case EMNETFLAG_DISCONNECT:
			time += STREAM_RESEND_DELAY;
			break;
		}
		
		break;
	
	case EMNETCLASS_STREAM:
		time += STREAM_RESEND_DELAY;
		cpacket.header |= EMNETFLAG_STREAMRESNT;
		break;
	}

	*packet = calloc(1, sizeof(struct out_packet_ll_t));
	memcpy(*packet, &cpacket, size);
	(*packet)->next_resend_time = time;
	(*packet)->size = size;
	return 1;
}


void send_ack(uint32_t index, struct sockaddr_in *addr)
{
	uint32_t header = EMNETFLAG_ACKWLDGEMNT | index;
	sendto(udp_socket, (char*)&header, EMNETHEADER_SIZE, 0, addr, sizeof(struct sockaddr_in));
}


void destroy_connection()
{
//	sigio_process &= ~SIGIO_PROCESS_NETWORK;
//	sigalrm_process &= ~SIGALRM_PROCESS_NETWORK;
	LL_REMOVE_ALL(struct in_packet_ll_t, &in_packet0);
	LL_REMOVE_ALL(struct out_packet_ll_t, &out_packet0);
	net_state = NETSTATE_DEAD;
}	


void output_text(char *text)
{
	buffer_cat_uint32(msg_buf, (uint32_t)MSG_TEXT);
	struct string_t *string = new_string_text(text);
	buffer_cat_uint32(msg_buf, (uint32_t)string);
}


void start_connection()
{
	output_text("Connecting to ");
	output_text(inet_ntoa(server_addr.sin_addr));
	output_text("...\n>");

	// send connect packet to server and keep sending it until it acknowledges

	cpacket.header = EMNETFLAG_CONNECT | EMNETCONNECT_MAGIC;
	output_cpacket();

	net_state = NETSTATE_CONNECTING;
}


void start_next_connection()
{
	memcpy(&server_addr, &next_addr, sizeof(struct sockaddr_in));
	start_connection();
//	sigio_process |= SIGIO_PROCESS_NETWORK;
//	sigalrm_process |= SIGALRM_PROCESS_NETWORK;
}


void process_ack(uint32_t index)
{
	struct out_packet_ll_t *temp;

	if(!out_packet0)
		return;

	if((out_packet0->packet.header & EMNETINDEX_MASK) == index)	// the packet is the first on the list
	{
		temp = out_packet0->next;
		free(out_packet0);
		out_packet0 = temp;
	}
	else
	{
		// search the rest of the list

		struct out_packet_ll_t *packet = out_packet0;

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

	if(net_state == NETSTATE_DISCONNECTING && !out_packet0)
	{
		destroy_connection();
		return;
	}

	if(net_state == NETSTATE_WAITING && !out_packet0)
	{
		destroy_connection();
		start_next_connection();
		return;
	}


	// record the time for timeout purposes

	last_net_time = get_double_time();
}


void process_connection_ack(uint32_t index)
{
	free(out_packet0);
	out_packet0 = NULL;

	next_read_index = next_process_index = 
		next_write_index = index;

	cpacket.header = next_write_index++;
	next_write_index %= EMNETINDEX_MAX + 1;
	cpacket.header |= EMNETCLASS_STREAM | EMNETFLAG_STREAMFIRST;
	
	net_state = NETSTATE_CONNECTED;

	buffer_cat_uint32(msg_buf, MSG_CONNECTION);
}


void process_disconnection()
{
	buffer_cat_uint32(msg_buf, MSG_DISCONNECTION);
}


void process_conn_lost()
{
	buffer_cat_uint32(msg_buf, MSG_CONNLOST);
}


void process_in_order_stream(struct in_packet_ll_t *end)
{
	struct buffer_t *buf = new_buffer();
	struct in_packet_ll_t *temp;
	int untimed = 0;
	
	if(in_packet0->stream_delivered)
		untimed = 1;
	
	uint32_t index = in_packet0->packet.header & EMNETINDEX_MASK;
	
	while(in_packet0 != end)
	{
		if(in_packet0->packet.header & EMNETFLAG_STREAMRESNT)
			untimed = 1;
			
		buffer_cat_buf(buf, (char*)&in_packet0->packet.payload, in_packet0->payload_size);

		temp = in_packet0;
		in_packet0 = in_packet0->next;
		free(temp);
	}

	if(in_packet0->packet.header & EMNETFLAG_STREAMRESNT)
		untimed = 1;
	
	buffer_cat_buf(buf, (char*)&in_packet0->packet.payload, in_packet0->payload_size);

	temp = in_packet0;
	in_packet0 = in_packet0->next;
	free(temp);

	if(untimed)
	{
		buffer_cat_uint32(msg_buf, MSG_STREAM_UNTIMED);
		buffer_cat_uint32(msg_buf, index);
	}
	else
	{
		buffer_cat_uint32(msg_buf, MSG_STREAM_TIMED);
		buffer_cat_uint32(msg_buf, index);
		buffer_cat_uint64(msg_buf, rdtsc());
	}
	
	buffer_cat_uint32(msg_buf, (uint32_t)buf);
}


void process_out_of_order_stream(struct in_packet_ll_t *start, struct in_packet_ll_t *end)
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
	
	buffer_cat_uint32(msg_buf, (uint32_t)buf);
	
	start->stream_delivered = 1;
}


void check_stream()
{
	struct in_packet_ll_t *cpacket = in_packet0;
	uint32_t nindex = next_process_index;

	check_first:
	
	if((cpacket->packet.header & EMNETINDEX_MASK) == nindex)
	{
		if((cpacket->packet.header & (EMNETCLASS_MASK | EMNETFLAG_MASK)) == EMNETFLAG_DISCONNECT)
		{
			net_state = NETSTATE_DISCONNECTED;
			last_net_time = get_double_time();
			process_disconnection();
			return;
		}
		
		
		// packet is definately stream first
		
		while(1)
		{
			if((cpacket->packet.header & EMNETINDEX_MASK) == next_read_index)
				next_read_index = (next_read_index + 1) % (EMNETINDEX_MAX + 1);
	
			if(cpacket->packet.header & EMNETFLAG_STREAMLAST)
			{
				process_in_order_stream(cpacket);
				
				cpacket = in_packet0;
				next_process_index = ++nindex;
				
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
					process_out_of_order_stream(cpacket, cpacket);
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
				process_out_of_order_stream(start_packet, cpacket);
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


void process_udp_data()
{
	struct packet_t packet;
	struct sockaddr_in recv_addr;
	

	// receive packet
	
	size_t addr_size = sizeof(struct sockaddr_in);
	int size = recvfrom(udp_socket, (char*)&packet, EMNETPACKET_SIZE, 0, 
		(struct sockaddr*)&recv_addr, &addr_size);
		
	
	// check packet has a header
	
	if(size < EMNETHEADER_SIZE)
		return;
	

	// see if packet is from server

	int from_server = recv_addr.sin_addr.s_addr == server_addr.sin_addr.s_addr &&
		recv_addr.sin_port == server_addr.sin_port;
	

	// get index/cookie
	
	uint32_t index = packet.header & EMNETINDEX_MASK;
	
	
	// process packet
	
	switch(packet.header & EMNETCLASS_MASK)
	{
	case EMNETCLASS_CONTROL:
		
		switch(packet.header & EMNETFLAG_MASK)
		{
		case EMNETFLAG_COOKIE:
		
			if(!from_server)
				break;				
		
			if(size != EMNETHEADER_SIZE)
				break;
		
			if(net_state != NETSTATE_CONNECTING)
				break;

			sendto(udp_socket, (char*)&packet, EMNETHEADER_SIZE, 0, 
				&server_addr, sizeof(struct sockaddr_in));
			
			output_text("<»");
			
			break;
	
			
		case EMNETFLAG_CONNECT:
			
			if(!from_server)
				break;				
		
			if(size != EMNETHEADER_SIZE)
				break;
		
			if(net_state != NETSTATE_CONNECTING)
				break;
	
			output_text("«\n");
			process_connection_ack(index);
			
			break;
			
			
		case EMNETFLAG_DISCONNECT:
			
			if(!from_server)
				break;
		
			if(size != EMNETHEADER_SIZE)
				break;

			if(net_state == NETSTATE_DEAD || net_state == NETSTATE_CONNECTING)
				break;
			
			send_ack(index, &recv_addr);
		
			if(net_state == NETSTATE_DISCONNECTING)
			{
				last_net_time = get_double_time();
				net_state = NETSTATE_DISCONNECTED;
				break;
			}
			
			if(net_state == NETSTATE_WAITING)
			{
				destroy_connection();
				start_next_connection();
				break;
			}
			
			if(net_state != NETSTATE_CONNECTED)
				break;
			
			if(index < next_read_index)
				index += EMNETINDEX_MAX + 1;
			
			if(index > next_read_index + OUT_OF_ORDER_RECV_AHEAD)
				break;
			
			if(insert_in_packet(&packet, size))
				check_stream(); // see if this packet has completed a stream
	
			break;
		
			
		case EMNETFLAG_ACKWLDGEMNT:
			
			if(!from_server)
				break;				
		
			if(size != EMNETHEADER_SIZE)
				break;
			
			if(net_state == NETSTATE_CONNECTING)
				break;
		
			process_ack(index);
			
			break;
		}
		
		break;

		
	case EMNETCLASS_STREAM:
		
		if(size == EMNETHEADER_SIZE)
			break;
		
		if(!from_server)
			break;
		
		if(net_state == NETSTATE_CONNECTING)
			break;
		
		send_ack(index, &recv_addr);
		
		if(net_state != NETSTATE_CONNECTED)
			break;
		
		if(index < next_read_index)
			index += EMNETINDEX_MAX + 1;
		
		if(index > next_read_index + OUT_OF_ORDER_RECV_AHEAD)
			break;
		
		if(insert_in_packet(&packet, size))
			check_stream(); // see if this packet has completed a stream
		
		break;
		
		
	case EMNETCLASS_MISC:
		
		switch(packet.header & EMNETFLAG_MASK)
		{
		case EMNETFLAG_SERVERINFO:
			break;
		}
		
		break;
	}	
}


void process_network_alarm()
{
	double time = get_double_time();
	struct out_packet_ll_t *packet = out_packet0;
	
	switch(net_state)
	{
	case NETSTATE_CONNECTING:
		
		if(time - last_net_time > CONNECTION_TIMEOUT)
		{
			output_text("\nFailed to connect.\n");
			destroy_connection();
			return;
		}
		
		if(time > packet->next_resend_time)
		{
			sendto(udp_socket, (char*)(packet), packet->size, 0, 
				&server_addr, sizeof(struct sockaddr_in));
				
			packet->next_resend_time += (floor((time - packet->next_resend_time) / 
				CONNECT_RESEND_DELAY) + 1.0) * CONNECT_RESEND_DELAY;
				
			output_text(">");
		}
		
		break;
		
	
	case NETSTATE_CONNECTED:
		
		// check if conn has timed out
		
		if(packet && time - last_net_time > CONNECTION_TIMEOUT)
		{
			process_conn_lost();
			destroy_connection();
			return;
		}
		
		
		// resend unacknowledged packets
	
		while(packet)
		{
			if(time > packet->next_resend_time)
			{
				sendto(udp_socket, (char*)(packet), packet->size, 0, 
					&server_addr, sizeof(struct sockaddr_in));
					
				packet->next_resend_time += (floor((time - packet->next_resend_time) / 
					STREAM_RESEND_DELAY) + 1.0) * STREAM_RESEND_DELAY;
			}
	
			packet = packet->next;
		}
			
		break;

		
	case NETSTATE_DISCONNECTING:
		
		// check if conn has timed out
		
		if(time - last_net_time > CONNECTION_TIMEOUT)
		{
			destroy_connection();
			break;
		}
		
		
		// resend unacknowledged packets
	
		while(packet)
		{
			if(time > packet->next_resend_time)
			{
				sendto(udp_socket, (char*)(packet), packet->size, 0, 
					&server_addr, sizeof(struct sockaddr_in));
					
				packet->next_resend_time += (floor((time - packet->next_resend_time) / 
					STREAM_RESEND_DELAY) + 1.0) * STREAM_RESEND_DELAY;
			}
	
			packet = packet->next;
		}
		
		break;
		
		
	case NETSTATE_DISCONNECTED:
		
		// check if conn has timed out
		
		if(time - last_net_time > CONNECTION_TIMEOUT)
		{
			destroy_connection();
			break;
		}
		
		break;
		
		
	case NETSTATE_WAITING:
		
		// check if conn has timed out
		
		if(time - last_net_time > CONNECTION_TIMEOUT)
		{
			destroy_connection();
			start_next_connection();
			break;
		}
		
		
		// resend unacknowledged packets
	
		while(packet)
		{
			if(time > packet->next_resend_time)
			{
				sendto(udp_socket, (char*)(packet), packet->size, 0, 
					&server_addr, sizeof(struct sockaddr_in));
					
				packet->next_resend_time += (floor((time - packet->next_resend_time) / 
					STREAM_RESEND_DELAY) + 1.0) * STREAM_RESEND_DELAY;
			}
	
			packet = packet->next;
		}
		
		break;
	}
}


void send_cpacket()
{
	sigio_process &= ~SIGIO_PROCESS_NETWORK;
	sigalrm_process &= ~SIGALRM_PROCESS_NETWORK;
	
	if(net_state != NETSTATE_CONNECTED)
		goto end;

	if(!output_cpacket())
	{
		destroy_connection();
		process_conn_lost();
		goto end;
	}

	cpacket.header = EMNETCLASS_STREAM | next_write_index++;
	next_write_index %= EMNETINDEX_MAX + 1;
		
	end:
	sigio_process |= SIGIO_PROCESS_NETWORK;
	sigalrm_process |= SIGALRM_PROCESS_NETWORK;
}


void net_emit_uint32(uint32_t val)
{
	net_in_use = 1;
	
	switch(cpacket_payload_size)
	{
	case EMNETPAYLOAD_SIZE:

		send_cpacket();

		*(uint32_t*)&cpacket.payload[0] = val;
		cpacket_payload_size = 4;

		break;


	case EMNETPAYLOAD_SIZE - 1:

		cpacket.payload[cpacket_payload_size] = ((uint8_t*)&val)[0];
		cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket();

		*(uint16_t*)&cpacket.payload[0] = *(uint16_t*)&((uint8_t*)&val)[1];
		cpacket.payload[2] = ((uint8_t*)&val)[3];
		cpacket_payload_size = 3;

		break;


	case EMNETPAYLOAD_SIZE - 2:

		*((uint16_t*)(void*)&cpacket.payload[cpacket_payload_size]) = ((uint16_t*)(void*)&val)[0];
		cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket();

		*(uint16_t*)(void*)&cpacket.payload[0] = ((uint16_t*)(void*)&val)[1];
		cpacket_payload_size = 2;

		break;


	case EMNETPAYLOAD_SIZE - 3:

		*(uint16_t*)(void*)&cpacket.payload[cpacket_payload_size] = ((uint16_t*)(void*)&val)[0];
		cpacket.payload[cpacket_payload_size + 2] = ((uint8_t*)&val)[2];
		cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket();

		cpacket.payload[0] = ((uint8_t*)&val)[3];
		cpacket_payload_size = 1;

		break;


	default:

		*(uint32_t*)&cpacket.payload[cpacket_payload_size] = val;
		cpacket_payload_size += 4;

		break;
	}
}


void net_emit_int(int val)
{
	net_emit_uint32(*(uint32_t*)&val);
}


void net_emit_float(float val)
{
	net_emit_uint32(*(uint32_t*)(void*)&val);
}


void net_emit_uint16(uint16_t val)
{
	net_in_use = 1;
	
	switch(cpacket_payload_size)
	{
	case EMNETPAYLOAD_SIZE:

		send_cpacket();

		*(uint16_t*)&cpacket.payload[0] = val;
		cpacket_payload_size = 2;

		break;


	case EMNETPAYLOAD_SIZE - 1:

		cpacket.payload[cpacket_payload_size] = ((uint8_t*)&val)[0];
		cpacket_payload_size = EMNETPAYLOAD_SIZE;
		send_cpacket();

		cpacket.payload[0] = ((uint8_t*)&val)[1];
		cpacket_payload_size = 1;

		break;


	default:

		*(uint16_t*)&cpacket.payload[cpacket_payload_size] = val;
		cpacket_payload_size += 2;

		break;
	}
}


void net_emit_uint8(uint8_t val)
{
	net_in_use = 1;
	
	switch(cpacket_payload_size)
	{
	case EMNETPAYLOAD_SIZE:
		send_cpacket();
		cpacket.payload[0] = val;
		cpacket_payload_size = 1;
		break;

	default:
		cpacket.payload[cpacket_payload_size] = val;
		cpacket_payload_size++;
		break;
	}
}


void net_emit_char(char val)
{
	net_emit_uint8(*(uint8_t*)&val);
}


void net_emit_string(char *cc)
{
	if(!cc)
		return;

	while(*cc)
		net_emit_char(*cc++);

	net_emit_char('\0');
}


void net_emit_buf(void *buf, int size)
{
	;
}


void net_emit_end_of_stream()
{
	cpacket.header |= EMNETFLAG_STREAMLAST;
	send_cpacket();
	cpacket.header |= EMNETFLAG_STREAMFIRST;
	cpacket_payload_size = 0;
	
	net_in_use = 0;
	if(net_fire_alarm)
	{
		raise(SIGALRM);
		net_fire_alarm = 0;
	}
}


void em_connect(char *addr)
{
	mask_sigs();
	uint16_t port = htons(NETWORK_PORT);
	uint32_t ip;
	
	if(!addr)
	{	
		ip = htonl(INADDR_LOOPBACK);
	}
	else
	{
		struct hostent *hostent;
		
		hostent = gethostbyname(addr);
		if(!hostent)
		{
			console_print("Couldn't get any host info; %s\n", strerror(h_errno));
			return;
		}
	
		ip = *(uint32_t*)hostent->h_addr;
	}

//	sigio_process &= ~SIGIO_PROCESS_NETWORK;
//	sigalrm_process &= ~SIGALRM_PROCESS_NETWORK;
	
	switch(net_state)
	{
	case NETSTATE_CONNECTED:
		console_print("Disconnecting...\n");
		net_state = NETSTATE_WAITING;
		cpacket.header = EMNETFLAG_DISCONNECT | (cpacket.header & EMNETINDEX_MASK);
		output_cpacket();
	
	case NETSTATE_WAITING:
	
		next_addr.sin_family = AF_INET;
		next_addr.sin_port = port;
		next_addr.sin_addr.s_addr = ip;
		
		break;
	
	
	case NETSTATE_DISCONNECTING:
		net_state = NETSTATE_WAITING;
		next_addr.sin_family = AF_INET;
		next_addr.sin_port = port;
		next_addr.sin_addr.s_addr = ip;
		break;
	
	case NETSTATE_DISCONNECTED:
	case NETSTATE_CONNECTING:
		destroy_connection();
	
	case NETSTATE_DEAD:

		server_addr.sin_family = AF_INET;
		server_addr.sin_port = port;
		server_addr.sin_addr.s_addr = ip;

		start_connection();
	}
	
//	sigio_process |= SIGIO_PROCESS_NETWORK;
//	sigalrm_process |= SIGALRM_PROCESS_NETWORK;
	unmask_sigs();
	
}


void em_disconnect(char *c)
{
	mask_sigs();
//	sigio_process &= ~SIGIO_PROCESS_NETWORK;
//	sigalrm_process &= ~SIGALRM_PROCESS_NETWORK;
	
	switch(net_state)
	{
	case NETSTATE_DEAD:
		console_print("Not connected.\n");
		return;
	
	case NETSTATE_DISCONNECTED:
		console_print("Not connected.\n");
		break;
	
	case NETSTATE_DISCONNECTING:
		console_print("Already disconnecting.\n");
		break;
		
	case NETSTATE_WAITING:
		net_state = NETSTATE_DISCONNECTING;
		break;
	
	case NETSTATE_CONNECTING:
		destroy_connection();
		return;

	case NETSTATE_CONNECTED:
		process_disconnection();
	
		cpacket.header = EMNETFLAG_DISCONNECT | (cpacket.header & EMNETINDEX_MASK);
		output_cpacket();
		
		net_state = NETSTATE_DISCONNECTING;
		break;
	}

//	sigio_process |= SIGIO_PROCESS_NETWORK;
//	sigalrm_process |= SIGALRM_PROCESS_NETWORK;
	unmask_sigs();
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
			client_libc_error("gethostname failure");
		}

		size *= 2;

		if(size > 65536)
		{
			free(hostname);
			client_error("Host name far too long!");
		}
			
		hostname = realloc(hostname, size);
	}

	console_print("Host: %s\n", hostname);

	struct hostent *hostent;
	hostent = gethostbyname(hostname);
	free(hostname);
	if(!hostent)
		client_libc_error("gethostbyname failure");

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
	
	// create udp socket

	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(udp_socket < 0)
		client_libc_error("Couldn't create socket");
	
	fcntl(udp_socket, F_SETOWN, getpid());
	fcntl(udp_socket, F_SETSIG, SIGRTMIN);
	fcntl(udp_socket, F_SETFL, O_ASYNC);
	
	// seed to rng
	
	uint64_t time = rdtsc();
	
	seed48((unsigned short int*)(void*)&time);

	sigio_process |= SIGIO_PROCESS_NETWORK;
	sigalrm_process |= SIGALRM_PROCESS_NETWORK;

	create_cvar_command("connect", em_connect);
	create_cvar_command("disconnect", em_disconnect);
}


void kill_network()
{
	if(net_state == NETSTATE_CONNECTED)
	{
		console_print("Disconnecting...\n");
		em_disconnect(NULL);
		
		// wait for disconnection
		
		sigset_t mask, oldmask;
		
		/* Set up the mask of signals to temporarily block. */
		sigemptyset(&mask);
		sigaddset(&mask, SIGRTMIN);
		sigaddset(&mask, SIGALRM);
		
		/* Wait for a signal to arrive. */
		sigprocmask(SIG_BLOCK, &mask, &oldmask);
		
		while(net_state != NETSTATE_DEAD)
			sigsuspend(&oldmask);
		
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}
}
