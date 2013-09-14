/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_MAP
#define _INC_MAP


#define EMERGENCE_FORKID		0
#define EMERGENCE_FORMATID	0

#define EMERGENCE_COMPILEDFORMATID	0

extern uint8_t map_active;
extern uint8_t map_saved;
extern uint8_t compiling;

extern struct string_t *map_filename;
extern struct string_t *map_path;

void run_space_menu();

void compile();

void clear_map();

void init_map();
void kill_map();

struct string_t *arb_rel2abs(char *path, char *base);
struct string_t *arb_abs2rel(char *path, char *base);


void run_open_dialog();
int run_not_saved_dialog();
int run_save_dialog();
int run_save_first_dialog();
int map_save();

void display_compile_dialog();
void destroy_compile_dialog();



#endif	// _INC_MAP
