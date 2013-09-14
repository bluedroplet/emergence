/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_CAMERON
#define _INC_CAMERON

extern int cameron_width, cameron_height;

void process_cameron();
void init_cameron();
void kill_cameron();
extern int cameron_pending;
void scale_cameron();
void draw_cameron();
void cameron_finished();

#endif	// _INC_CAMERON
