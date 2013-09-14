/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

int load_map(char *name);
void render_map();
void init_map_cvars();
int game_process_load_map();
void game_process_map_downloaded();
void game_process_map_download_failed();
void reload_map();

extern struct string_t *map_name;
extern int downloading_map;
extern int map_loaded;
extern int map_size;

#ifdef _FILEINFO_H
extern uint8_t map_hash[FILEINFO_DIGEST_SIZE];
#endif
