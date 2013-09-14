/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

void init_x();
void process_x();
extern int x_fd;
void update_frame_buffer();
void create_x_cvars();
void kill_x();
extern int x_render_pipe[2];
extern int use_x_mouse;
extern int vid_mode;
