#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

#include "../../common/string.h"
#include "../../common/buffer.h"
#include "../../common/cvar.h"
#include "../../shared/network.h"
#include "main.h"
#include "pipe.h"
#include "console.h"
#include "game.h"
#include "network.h"


HANDLE network_thread_handle = NULL;
HANDLE udp_data_event = NULL;
HANDLE network_timer_event = NULL;
HANDLE to_net_pipe = NULL, from_net_pipe = NULL;

SOCKET udp_socket = INVALID_SOCKET;


int net_state = NETSTATE_DEAD;

SOCKADDR_IN server_addr;

packet_t cpacket;	// the packet currently being constructed
packet_t *outpacket0;	// unacknowledged sent packets

packet_t *inpacket0;	// unprocessed received packets in index order

dword nextindex;	// the index that the next packet to be processed should have


#define NETWORK_SEND_PACKET		1
#define NETWORK_RECV_MESSAGE	2
#define NETWORK_CONNECTED		3
#define NETWORK_DISCONNECTING	4

void add_packet(packet_t **packet0, packet_t *packet)
{
	// check packet0 is not NULL

	if(!packet0)
		return;


	// if *packet0 is NULL, then create new packet here

	if(!*packet0)
	{
		*packet0 = new packet_t;

		memcpy(*packet0, packet, packet->size + 8);

		(*packet0)->next = NULL;

		return;
	}
	 

	// create new packet at end of list

	packet_t *cpacket = *packet0;

	while(cpacket->next)
		cpacket = cpacket->next;

	cpacket->next = new packet_t;

	cpacket = cpacket->next;

	memcpy(cpacket, packet, packet->size + 8);

	cpacket->next = NULL;
}


void insert_packet(packet_t **packet0, packet_t *packet)
{
	packet_t *temp;

	// check packet0 is not NULL

	if(!packet0)
		return;


	// if *packet0 is NULL, then create new packet here

	if(!*packet0)
	{
		*packet0 = new packet_t;

		memcpy(*packet0, packet, packet->size + 8);

		(*packet0)->next = NULL;
		return;
	}

	
	if((*packet0)->index == packet->index)
		return; // the packet is already present in the list
	
	if((*packet0)->index > packet->index)
	{
		temp = *packet0;
		*packet0 = new packet_t;
		memcpy(*packet0, packet, packet->size + 8);
		(*packet0)->next = temp;
		return;
	}

	
	// search through the rest of the list
	// to find the packet before the first
	// packet to have an index greater
	// than index


	packet_t **cpacket = packet0;

	
	while((*cpacket)->next)
	{
		if((*cpacket)->next->index == packet->index)
			return; // the packet is already present in the list

		if((*cpacket)->next->index > packet->index)
		{
			temp = (*cpacket)->next;
			(*cpacket)->next = new packet_t;
			*cpacket = (*cpacket)->next;

			memcpy(*cpacket, packet, packet->size + 8);

			(*cpacket)->next = temp;

			return;
		}

		*cpacket = (*cpacket)->next;
	}

	// we have reached the end of the list and not found
	// a packet that has an index greater than index

	(*cpacket)->next = new packet_t;
	*cpacket = (*cpacket)->next;

	memcpy(*cpacket, packet, packet->size + 8);

	(*cpacket)->next = NULL;
}



//
// NETWORK THREAD FUNCTIONS
//


void process_send_packet(buffer *buf)
{
	packet_t cpacket;

	memcpy(&cpacket, &buf->buf[buf->pos], 8);

	buf->pos += 8;

	memcpy(cpacket.data, &buf->buf[buf->pos], cpacket.size);

	buf->pos += cpacket.size;

	sendto(udp_socket, (char*)&cpacket, cpacket.size + 8, 0, 
		(SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN));

	add_packet(&outpacket0, &cpacket);
}


void process_pipe_data()
{
	buffer *buf = thread_pipe_recv(to_net_pipe);

	int stop = 0;

	while(!stop)
	{
		switch(buf->read_int())
		{
		case NETWORK_SEND_PACKET:
			process_send_packet(buf);
			break;

		case BUF_EOB:
			stop = 1;
			break;
		}
	}

	delete buf;
}


void send_acknowledgement(dword index)
{
	packet_t packet;

	packet.index = index;
	packet.flags = NFNETFLAG_ACKWLDGEMNT;

	sendto(udp_socket, (char*)&packet, 6, 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN));
}


void acknowledgement_received(dword index)
{
	// remove packet with that index from outpacket0

	packet_t *temp;

	if(!outpacket0)
		return;

	if(outpacket0->index == index)	// the packet is the first on the list
	{
		temp = outpacket0->next;
		delete outpacket0;
		outpacket0 = temp;
	}
	else
	{

		// search the rest of the list

		packet_t *packet = outpacket0;

		while(packet->next)
		{
			if(packet->next->index == index)
			{
				temp = packet->next->next;
				delete packet->next;
				packet->next = temp;
				break;
			}

			packet = packet->next;
		}
	}

	// if the connection has been severed this side and
	// all packets have been acked then exit thread
	
	if(net_state == NETSTATE_DISCONNECTING && !outpacket0)
		ExitThread(0);
}


void connection_acknowledgement_received()
{
	packet_t *temp;

	if(outpacket0)
	{
		if((outpacket0->flags & NFNETFIELD_TYPE) == NFNETFLAG_CHALLENGE)	// the packet is the first on the list
		{
			temp = outpacket0->next;
			delete outpacket0;
			outpacket0 = temp;
		}
	}

	nextindex = 0;

	net_state = NETSTATE_CONNECTED;

	buffer buf((int)NETWORK_CONNECTED);
	thread_pipe_send(from_net_pipe, &buf);
}


void pipe_message()
{
	packet_t *temp;
	buffer buf((int)NETWORK_RECV_MESSAGE);

	while(!(inpacket0->flags & NFNETFLAG_LASTPACKET))
	{
		buf.cat((char*)inpacket0->data, inpacket0->size);

		temp = inpacket0;
		inpacket0 = inpacket0->next;
		delete temp;
	}

	buf.cat((char*)inpacket0->data, inpacket0->size);

	temp = inpacket0;
	inpacket0 = inpacket0->next;
	delete temp;

	buf.cat((int)NFNETMSG_ENDOFSTREAM);

	thread_pipe_send(from_net_pipe, &buf);
}


void process_server_disconnection()
{
	net_state = NETSTATE_DISCONNECTING;
	buffer buf((int)NETWORK_DISCONNECTING);
	thread_pipe_send(from_net_pipe, &buf);
	ExitThread(0);
}

void check_stream()
{
	packet_t *packet = inpacket0;	// the unprocessed packet with the lowest index
	dword cindex = nextindex;		// the index that the next packet to be processed should have

	while(packet)
	{
		if(packet->index != cindex)
			break;	// this packet breaks the contigious stream

		if((packet->flags & NFNETFIELD_TYPE) == NFNETFLAG_DISCONNECT)
		{
			process_server_disconnection();
			return;
		}

		if(packet->flags & NFNETFLAG_LASTPACKET)	// we are on the last packet in a stream
		{
			pipe_message();

			nextindex = cindex + 1;

			check_stream();

			return;
		}

		packet = packet->next;
		cindex++;
	}
}


void process_udp_data()
{
	packet_t packet;
	SOCKADDR_IN recv_addr;


	// receive packet and address

	int size = sizeof(SOCKADDR_IN);
	recvfrom(udp_socket, (char*)&packet, 512, 0, (SOCKADDR*)&recv_addr, &size);


	// ensure packet is from server

	if(recv_addr.sin_addr.S_un.S_addr != server_addr.sin_addr.S_un.S_addr)
		return;


	// process packet

	switch(packet.flags & NFNETFIELD_TYPE)
	{
	case NFNETFLAG_CHALLENGEACK:

		if(net_state != NETSTATE_CONNECTING)
			break;

		connection_acknowledgement_received();

		break;


	case NFNETFLAG_STREAMDATA:
	case NFNETFLAG_DISCONNECT:

		// if the connection has not been acknowledged by the server,
		// then don't acknowledge stream packets as they will be resent

		if(net_state != NETSTATE_CONNECTED)
			break;

		send_acknowledgement(packet.index);

		if(packet.index < nextindex)
			break;

		insert_packet(&inpacket0, &packet);

		check_stream(); // see if this packet has completed a stream

		break;


	case NFNETFLAG_ACKWLDGEMNT:

		if(net_state != NETSTATE_CONNECTED && net_state != NETSTATE_DISCONNECTING)
			break;

		acknowledgement_received(packet.index);

		break;
	}
}


void process_network_tick()
{
	// resend all unacknowledged packets

	packet_t *packet = outpacket0;

	while(packet)
	{
		sendto(udp_socket, (char*)packet, packet->size + 8, 0, 
			(SOCKADDR*)&server_addr, sizeof(SOCKADDR_IN));

		packet = packet->next;
	}
}


DWORD WINAPI network_thread(void *arg)
{
	HANDLE events[3] = {to_net_pipe, udp_data_event, network_timer_event};

	SetThreadPriority(network_thread_handle, THREAD_PRIORITY_HIGHEST);

	while(1)
	{
		int event = WaitForMultipleObjects(3, events, 0, INFINITE);

		switch(event -= WAIT_OBJECT_0)
		{
		case 0:
			process_pipe_data();
			break;

		case 1:
			process_udp_data();
			break;

		case 2:
			process_network_tick();
			break;
		}
	}

	return 1;
}


//
// SUB INTERFACE FUNCTIONS
//


void send_cpacket()
{
	buffer buf((int)NETWORK_SEND_PACKET);
	buf.cat((char*)&cpacket, cpacket.size + 8);

	thread_pipe_send(to_net_pipe, &buf);

	cpacket.index++;
	cpacket.flags = NFNETFLAG_STREAMDATA;
	cpacket.size = 0;
}


//
// INTERFACE FUNCTIONS
//


void net_write_dword(dword val)
{
	switch(cpacket.size)
	{
	case 501:

		*(word*)&cpacket.data[cpacket.size] = ((word*)&val)[0];
		*(byte*)&cpacket.data[cpacket.size + 2] = ((byte*)&val)[2];

		cpacket.size = 512;

		send_cpacket();

		*(byte*)&cpacket.data[0] = ((byte*)&val)[3];

		cpacket.size = 1;

		break;


	case 502:

		*(word*)&cpacket.data[cpacket.size] = ((word*)&val)[0];

		cpacket.size = 512;

		send_cpacket();

		*(word*)&cpacket.data[0] = ((word*)&val)[1];

		cpacket.size = 2;

		break;


	case 503:

		*(byte*)&cpacket.data[cpacket.size] = ((byte*)&val)[0];

		cpacket.size = 512;

		send_cpacket();

		*(byte*)&cpacket.data[0] = ((byte*)&val)[1];
		*(word*)&cpacket.data[1] = ((word*)&val)[1];

		cpacket.size = 3;

		break;


	case 504:

		send_cpacket();

		*(dword*)&cpacket.data[0] = val;
		cpacket.size = 4;

		break;


	default:

		*(dword*)&cpacket.data[cpacket.size] = val;

		cpacket.size += 4;

		break;
	}
}


void net_write_int(int val)
{
	net_write_dword(*(dword*)&val);
}


void net_write_word(word val)
{
	switch(cpacket.size)
	{
	case 503:

		*(byte*)&cpacket.data[cpacket.size] = ((byte*)&val)[0];

		cpacket.size = 512;

		send_cpacket();

		*(byte*)&cpacket.data[0] = ((byte*)&val)[1];

		cpacket.size = 1;

		break;


	case 504:

		send_cpacket();

		*(word*)&cpacket.data[0] = val;
		cpacket.size = 2;

		break;


	default:

		*(word*)&cpacket.data[cpacket.size] = val;

		cpacket.size += 2;

		break;
	}
}


void net_write_byte(byte val)
{
	switch(cpacket.size)
	{
	case 504:

		send_cpacket();

		*(byte*)&cpacket.data[0] = val;
		cpacket.size = 1;

		break;


	default:

		*(byte*)&cpacket.data[cpacket.size] = val;

		cpacket.size += 1;

		break;
	}
}


void net_write_char(char val)
{
	net_write_byte(*(byte*)&val);
}


void net_write_float(float val)
{
	net_write_dword(*(dword*)&val);
}


void net_write_string(string *str)
{
	char *cc = str->text;

	while(*cc)
		net_write_char(*cc++);

	net_write_char('\0');
}


void finished_writing()
{
	cpacket.flags |= NFNETFLAG_LASTPACKET;

	send_cpacket();
}


void connect(char *addr)
{
	if(net_state != NETSTATE_DEAD)
	{
		console_print("Already connected.\n");
		return;
	}

	// create udp socket

	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(udp_socket == INVALID_SOCKET)
		shutdown();

	SOCKADDR_IN sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = 0;	// let os define local port
	sockaddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if(bind(udp_socket, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
		shutdown();

	udp_data_event = CreateEvent(NULL, 0, 0, NULL);
	if(udp_data_event == NULL)
		shutdown();

	if(WSAEventSelect(udp_socket, udp_data_event, FD_READ) == SOCKET_ERROR)
		shutdown();

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(28960);

	network_timer_event = CreateWaitableTimer(NULL, 0, NULL);
	if(!network_timer_event)
		shutdown();

	DWORDLONG StartTime = 0;
	if(!SetWaitableTimer(network_timer_event, (LARGE_INTEGER*)&StartTime, 20, NULL, NULL, 0))
		shutdown();

	console_print("UDP initialized.\n");

	in_addr ipaddr;

	ipaddr.S_un.S_addr = inet_addr(addr);

	if(ipaddr.S_un.S_addr == 0 || ipaddr.S_un.S_addr == 4294967295)
		server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.0.12");
	else
		server_addr.sin_addr = ipaddr;

	string cline("Connecting to ");
	cline.cat(inet_ntoa(server_addr.sin_addr));
	cline.cat("...\n");

	console_print(cline.text);


	// send connect packet to server and keep sending it until it acknowledges

	cpacket.index = 0;
	cpacket.size = 0;
	cpacket.flags = NFNETFLAG_CHALLENGE;

	send_cpacket();

	cpacket.index = 0;
	cpacket.flags = NFNETFLAG_STREAMDATA;
	cpacket.size = 0;

	net_state = NETSTATE_CONNECTING;

	DWORD thread_id;
	network_thread_handle = CreateThread(NULL, 0, network_thread, NULL, 0, &thread_id);
	
	if(network_thread_handle == NULL)
	{
		shutdown();
	}
}


void init_network()
{
	// init sockets

	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
		shutdown();


	// get local address(es)

	char hostname[64];
	gethostname(hostname, 64);

	HOSTENT *hostent;
	hostent = gethostbyname(hostname);
	if(hostent == NULL)
		shutdown();

	in_addr **addr = (in_addr**)hostent->h_addr_list;
	if(!addr)
		shutdown();

	string addr_list(inet_ntoa(**addr));

	addr++;

	int multiple = 0;

	while(*addr)
	{
		addr_list.cat("; ");
		addr_list.cat(inet_ntoa(**addr));

		addr++;
		multiple = 1;
	}

	create_cvar_string("ipaddr", addr_list.text, CVAR_PROTECTED);

	string cline;

	if(multiple)
		cline.cat("IP addresses: ");
	else
		cline.cat("IP address: ");

	cline.cat(&addr_list);
	cline.cat("\n");

	console_print(cline.text);

	net_state = NETSTATE_DEAD;

	to_net_pipe = create_thread_pipe();
	from_net_pipe = create_thread_pipe();

	create_cvar_command("connect", connect);
	create_cvar_command("disconnect", disconnect);
}


void kill_network()
{
	disconnect(NULL);

	WSACleanup();
}


buffer *msgbuf;


int net_read_int()
{
	return msgbuf->read_int();
}


dword net_read_dword()
{
	return msgbuf->read_dword();
}


word net_read_word()
{
	return msgbuf->read_word();
}


byte net_read_byte()
{
	return msgbuf->read_byte();
}


float net_read_float()
{
	return msgbuf->read_float();
}


string *net_read_string()
{
	return msgbuf->read_string();
}


void process_disconnection()
{
	if(WaitForSingleObject(network_thread_handle, 10000) == WAIT_TIMEOUT)
		TerminateThread(network_thread_handle, 0);

	CloseHandle(network_thread_handle);
	network_thread_handle = NULL;

	if(udp_socket != INVALID_SOCKET)
	{
		closesocket(udp_socket);
		udp_socket = INVALID_SOCKET;
	}

	CloseHandle(udp_data_event);
	udp_data_event = NULL;

	CloseHandle(network_timer_event);
	network_timer_event = NULL;
	packet_t *temp;
	
	while(inpacket0)
	{
		temp = inpacket0->next;
		delete inpacket0;
		inpacket0 = temp;
	}

	while(outpacket0)
	{
		temp = outpacket0->next;
		delete outpacket0;
		outpacket0 = temp;
	}

	net_state = NETSTATE_DEAD;
}


void disconnect(char*)
{
	if(net_state != NETSTATE_CONNECTED)
		return;

	net_state = NETSTATE_DISCONNECTING;

	cpacket.size = 0;
	cpacket.flags = NFNETFLAG_DISCONNECT;

	send_cpacket();

	buffer buf((int)NETWORK_DISCONNECTING);
	thread_pipe_send(from_net_pipe, &buf);

	process_disconnection();
}


void process_network_message()
{
	buffer *buf = thread_pipe_recv(from_net_pipe);

	int stop = 0;

	while(!stop)
	{
		switch(buf->read_int())
		{
		case NETWORK_RECV_MESSAGE:
			msgbuf = buf;
			game_process_stream();
			break;

		case NETWORK_CONNECTED:
			console_print("Connected.\n");
			game_process_connection();
			break;

		case NETWORK_DISCONNECTING:
			console_print("Disconnected.\n");
			process_disconnection();
			game_process_disconnection();
			break;

		case BUF_EOB:
			stop = 1;
			break;
		}
	}
}
