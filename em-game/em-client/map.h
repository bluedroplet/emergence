int load_map(char *name);
void render_map();
void init_map_cvars();
int game_process_load_map();
void game_process_map_downloaded();
void game_process_map_download_failed();
void reload_map();

extern struct string_t *map_name;
extern int downloading_map;
