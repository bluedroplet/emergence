void init_network();
void kill_network();


void net_emit_uint32(uint32_t val);
void net_emit_int(int val);
void net_emit_float(float val);
void net_emit_uint16(uint16_t val);
void net_emit_uint8(uint8_t val);
void net_emit_char(char val);
void net_emit_string(char *cc);
void net_emit_buf(void *buf, int size);
void net_emit_end_of_stream();



void em_disconnect(char*);

#define NETSTATE_DEAD			0	// there is no connection
#define NETSTATE_CONNECTING		1	// a connection has been requested on the server
#define NETSTATE_CONNECTED		2	// the server has acknowledged the connection
#define NETSTATE_DISCONNECTING	3	// the client has requested a disconnection
#define NETSTATE_DISCONNECTED	4	// the server has requested a disconnection
#define NETSTATE_WAITING		5	// waiting for disconnection so connection to another server can occur

extern int net_state;

extern int udp_socket;
void process_network_alarm();
void process_udp_data();

extern int net_in_use;
extern int net_fire_alarm;
