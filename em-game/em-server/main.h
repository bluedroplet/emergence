
#ifndef _INC_MAIN
#define _INC_MAIN

void server_shutdown();
void server_error(const char *fmt, ...);
void server_libc_error(const char *fmt, ...);
void init();


#define MSG_COMMAND				0
#define MSG_UPDATE_GAME			1

void process_msg_buf(struct buffer_t *msg_buf);
extern struct buffer_t *msg_buf;
void main_thread();

#endif // _INC_MAIN
