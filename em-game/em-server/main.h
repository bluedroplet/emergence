
#ifndef _INC_MAIN
#define _INC_MAIN

void server_shutdown();
void server_error(const char *fmt, ...);
void server_libc_error(const char *fmt, ...);
void init();


#define MSG_CONNECTION			0
#define MSG_DISCONNECTION		1
#define MSG_CONNLOST			2
#define MSG_STREAM_TIMED		4
#define MSG_STREAM_UNTIMED		5
#define MSG_STREAM_TIMED_OOO	6
#define MSG_STREAM_UNTIMED_OOO	7
#define MSG_COMMAND				8
#define MSG_UPDATE_GAME			9

void process_msg_buf(struct buffer_t *msg_buf);
extern struct buffer_t *msg_buf;

#endif // _INC_MAIN
