/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void init_alarm();
void kill_alarm();
void create_alarm_listener(void (*func)());
//void destroy_alarm_listener(int read_fd);
