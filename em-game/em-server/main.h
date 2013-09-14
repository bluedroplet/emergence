/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

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

void process_session_accepted(uint32_t conn);
void process_session_declined(uint32_t conn);
void process_session_error(uint32_t conn);


#endif // _INC_MAIN
