#define RCCON_OUT		0
#define RCCON_ENTERING	1
#define RCCON_IN		2

extern int rconing;
void init_rcon();
void rcon_command(char *text);


void process_inrcon();
void process_outrcon();
