
/*
changes to this file must not break backward compatibility
*/


#define EMNETINDEX_MAX					0x07ffffff
#define EMNETINDEX_MASK					EMNETINDEX_MAX
#define EMNETCOOKIE_MASK				EMNETINDEX_MASK



#define EMNETCLASS_MASK				(0x3 << 30)
#define EMNETFLAG_MASK					(0x7 << 27)


#define EMNETCLASS_CONTROL			(0x0 << 30)

#define EMNETFLAG_CONNECT				(0x0 << 27)		// sent by client to start connection handshake
														// and subsequently returned by server after cookie verification
#define EMNETFLAG_COOKIE				(0x1 << 27)		// sent by server
														// and subsequently returned by client
#define EMNETFLAG_DISCONNECT			(0x2 << 27)		// sent by either server or client
#define EMNETFLAG_ACKWLDGEMNT			(0x3 << 27)		// acknowledgement of EMNETFLAG_STREAMDATA* or EMNETFLAG_DISCONNECT


#define EMNETCLASS_STREAM			(0x1 << 30)		// sent by either server or client

#define EMNETFLAG_STREAMFIRST			(0x1 << 27)
#define EMNETFLAG_STREAMLAST			(0x2 << 27)
#define EMNETFLAG_STREAMRESNT			(0x4 << 27)


#define EMNETCLASS_MISC				(0x3 << 30)

#define EMNETFLAG_PING					(0x0 << 27)		// with cookie
#define EMNETFLAG_PONG					(0x1 << 27)		// with cookie

#define EMNETFLAG_SERVERINFO			(0x7 << 27)		// info about server


#define EMNETCONNECT_MAGIC				0x02708d11



#define EMNETPACKET_MAXSIZE				512
#define EMNETHEADER_SIZE				4
#define EMNETPAYLOAD_MAXSIZE			508



struct packet_t
{
	uint32_t header;
	uint8_t payload[EMNETPAYLOAD_MAXSIZE];
};


#define EMNET_PORT				45420

void init_network();
void kill_network();

struct string_t *get_text_addr(void *conn);


void net_emit_uint32(uint32_t temp_conn, uint32_t val);
void net_emit_int(uint32_t temp_conn, int val);
void net_emit_float(uint32_t temp_conn, float val);
void net_emit_uint16(uint32_t temp_conn, uint16_t val);
void net_emit_uint8(uint32_t temp_conn, uint8_t val);
void net_emit_char(uint32_t temp_conn, char val);
void net_emit_string(uint32_t temp_conn, char *string);
void net_emit_buf(uint32_t temp_conn, void *buf, int size);
void net_emit_end_of_stream(uint32_t temp_conn);

#ifdef EMCLIENT
void em_connect(char *addr);
#endif

void em_disconnect(uint32_t conn);

#define NETMSG_CONNECTING			0
#define NETMSG_COOKIE_ECHOED		1
#define NETMSG_CONNECTION			2
#define NETMSG_CONNECTION_FAILED	3
#define NETMSG_DISCONNECTION		4
#define NETMSG_CONNLOST				5
#define NETMSG_STREAM_TIMED			6
#define NETMSG_STREAM_UNTIMED		7
#define NETMSG_STREAM_TIMED_OOO		8
#define NETMSG_STREAM_UNTIMED_OOO	9

extern int net_in_pipe[2];
extern int net_out_pipe[2];
