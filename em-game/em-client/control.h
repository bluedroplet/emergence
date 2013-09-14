/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void process_keypress(uint32_t key, int state);
void process_button(uint32_t control, int state);
void process_axis(uint32_t control, float val);
void process_control_alarm();
void create_control_cvars();
void init_control();
void kill_control();

#if defined _INC_STDIO || defined _STDIO_H
void dump_bindings(FILE *file);
#endif
