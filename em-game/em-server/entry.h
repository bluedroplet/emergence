/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void go_daemon();
void terminate_process();
extern int as_daemon;

extern int arg_max_players;
extern struct string_t *arg_script;
extern struct string_t *arg_map;
