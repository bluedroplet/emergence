/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _DOWNLOAD_H
#define _DOWNLOAD_H

int download_map(char *map_name);
void stop_downloading_map();
void init_download();
void kill_download();
void process_download_out_pipe();
void render_map_downloading();
extern int download_out_pipe[2];

#endif
