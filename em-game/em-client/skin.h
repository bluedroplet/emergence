/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_SKIN
#define _INC_SKIN

struct skin_t
{
	uint32_t index;
	struct string_t *name;
	
	struct surface_t *craft;
	struct surface_t *rocket_launcher;
	struct surface_t *minigun;
	struct surface_t *plasma_cannon;
	struct surface_t *plasma;
	struct surface_t *rocket;
		
	struct skin_t *next;
};


int game_process_load_skin();

struct skin_t *get_skin(uint32_t index);
void reload_skins();
void flush_all_skins();
void clear_skins();
void init_skin();

struct surface_t *skin_get_craft_surface(uint32_t index);
struct surface_t *skin_get_rocket_launcher_surface(uint32_t index);
struct surface_t *skin_get_minigun_surface(uint32_t index);
struct surface_t *skin_get_plasma_cannon_surface(uint32_t index);

	

#endif	// _INC_SKIN
