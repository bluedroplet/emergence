void init_network();
void kill_network();


void net_write_dword(dword val);
void net_write_int(int val);
void net_write_word(word val);
void net_write_byte(byte val);
void net_write_float(float val);
void net_write_string(string *str);
void finished_writing();


// only to be called from game_process_stream
int net_read_int();
dword net_read_dword();
word net_read_word();
byte net_read_byte();
float net_read_float();
string *net_read_string();


void process_network_message();

#ifdef _INC_WINDOWS
extern HANDLE from_net_pipe;
#endif

void disconnect(char*);

#define NETSTATE_DEAD			0	// there is no connection
#define NETSTATE_CONNECTING		1	// a connection has been requested on the server
#define NETSTATE_CONNECTED		2	// the server has acknowledged the connection
#define NETSTATE_DISCONNECTING	3	// the client has requested a disconnection

extern int net_state;
