struct server_t
{
	uint32_t ip;
	uint16_t port;
	time_t time;

	struct server_t *next;
	
};

extern struct server_t *rumoured_servers;


void load_rumoured_servers();
void save_rumoured_servers();
int add_new_server(struct server_t **server0, struct sockaddr_in *sockaddr, time_t t);
int count_servers();
