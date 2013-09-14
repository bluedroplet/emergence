/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void init_key();
void kill_key();
int key_create_session();
void process_key_out_pipe();

extern int key_out_pipe[2];
