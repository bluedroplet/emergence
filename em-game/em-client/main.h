/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void init();
void client_shutdown();
void client_error(const char *fmt, ...);
void client_libc_error(const char *fmt, ...);
void main_thread();

#ifdef _INC_NFBUFFER
void process_messages(struct buffer_t *msg_buf);
#endif


#define MSG_CONNECTION			0
#define MSG_DISCONNECTION		1
#define MSG_CONNLOST			2
#define MSG_STREAM_TIMED		3
#define MSG_STREAM_UNTIMED		4
#define MSG_STREAM_TIMED_OOO	5
#define MSG_STREAM_UNTIMED_OOO	6
#define MSG_KEYPRESS			7
#define MSG_UIFUNC				8
#define MSG_TEXT				9
#define MSG_RENDER				10
#define MSG_PING				11
