
#define SIGIO_PROCESS_NETWORK	0x01
#define SIGIO_PROCESS_INPUT		0x02
#define SIGIO_PROCESS_X			0x04

#define SIGALRM_PROCESS_NETWORK	0x01
#define SIGALRM_PROCESS_CONTROL	0x02

void init_user();
void terminate_process();

extern volatile int sigio_process;
extern volatile int sigalrm_process;
extern struct buffer_t *msg_buf;

void mask_sigs();
void unmask_sigs();
