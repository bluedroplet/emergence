/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_MAIN
#define _INC_MAIN


void update_client_area();
void destroy_window();

extern int vid_width, vid_height;
extern struct surface_t *s_backbuffer;

#ifdef __GTK_H__
extern GtkWidget *window;
#endif

#endif	// _INC_MAIN
