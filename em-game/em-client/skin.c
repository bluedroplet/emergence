#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../common/llist.h"
#include "../gsub/gsub.h"
#include "entry.h"
#include "skin.h"
#include "console.h"
#include "rotate.h"
#include "game.h"

struct skin_t *skin0 = NULL;

int game_process_load_skin(struct buffer_t *stream)
{
	struct skin_t skin;
		
	struct string_t *skin_name = buffer_read_string(stream);
	skin.index = buffer_read_uint32(stream);
		
	struct string_t *filename = new_string_string(emergence_home_dir);
	string_cat_text(filename, "/skins/");	
	string_cat_string(filename, skin_name);
	string_cat_text(filename, ".skin.cache");
	
	gzFile gzfile = gzopen(filename->text, "rb");
	if(gzfile)
	{
		skin.craft = gzread_raw_surface(gzfile);
		skin.plasma_cannon = gzread_raw_surface(gzfile);
		skin.minigun = gzread_raw_surface(gzfile);
		skin.rocket_launcher = gzread_raw_surface(gzfile);
		gzclose(gzfile);
		LL_ADD(struct skin_t, &skin0, &skin);
		free_string(skin_name);
		free_string(filename);
		return 1;
	}
	
	free_string(filename);
	
	filename = new_string_string(emergence_home_dir);
	string_cat_text(filename, "/skins/");	
	string_cat_string(filename, skin_name);
	string_cat_text(filename, ".skin");
	
	gzfile = gzopen(filename->text, "rb");
	if(!gzfile)
	{
		console_print("Could not load skin: %s\n", filename->text);
		free_string(filename);
		free_string(skin_name);
		return 0;
	}
	
	free_string(filename);
	

	struct rotate_t craft_rot;
	craft_rot.in_surface = gzread_raw_surface(gzfile);
	craft_rot.out_surface = &skin.craft;
	craft_rot.next = NULL;
	
	do_rotate(&craft_rot, 40, 40, ROTATIONS);
	
	struct rotate_t rocket_launcher_rot;
	rocket_launcher_rot.in_surface = gzread_raw_surface(gzfile);
	rocket_launcher_rot.out_surface = &skin.rocket_launcher;
	rocket_launcher_rot.next = NULL;
	
	struct rotate_t minigun_rot;
	minigun_rot.in_surface = gzread_raw_surface(gzfile);
	minigun_rot.out_surface = &skin.minigun;
	minigun_rot.next = &rocket_launcher_rot;
	
	struct rotate_t plasma_cannon_rot;
	plasma_cannon_rot.in_surface = gzread_raw_surface(gzfile);
	plasma_cannon_rot.out_surface = &skin.plasma_cannon;
	plasma_cannon_rot.next = &minigun_rot;
	
	do_rotate(&plasma_cannon_rot, 25, 25, ROTATIONS);
	
	convert_surface_to_16bitalpha8bit(skin.craft);
	convert_surface_to_16bitalpha8bit(skin.rocket_launcher);
	convert_surface_to_16bitalpha8bit(skin.minigun);
	convert_surface_to_16bitalpha8bit(skin.plasma_cannon);
	
	gzclose(gzfile);
	LL_ADD(struct skin_t, &skin0, &skin);
	
	filename = new_string_string(emergence_home_dir);
	string_cat_text(filename, "/skins/");	
	string_cat_string(filename, skin_name);
	string_cat_text(filename, ".skin.cache");
	
	gzfile = gzopen(filename->text, "wb9");
	if(!gzfile)
		return 0;

	gzwrite_raw_surface(gzfile, skin.craft);
	gzwrite_raw_surface(gzfile, skin.plasma_cannon);
	gzwrite_raw_surface(gzfile, skin.minigun);
	gzwrite_raw_surface(gzfile, skin.rocket_launcher);
	gzclose(gzfile);
	
	free_string(filename);
	
	free_string(skin_name);
	return 1;
}


struct surface_t *skin_get_craft_surface(uint32_t index)
{
	struct skin_t *skin = skin0;
		
	while(skin)
	{
		if(skin->index == index)
			return skin->craft;
		
		skin = skin->next;
	}
	
	return NULL;
}


struct surface_t *skin_get_rocket_launcher_surface(uint32_t index)
{
	struct skin_t *skin = skin0;
		
	while(skin)
	{
		if(skin->index == index)
			return skin->rocket_launcher;
		
		skin = skin->next;
	}
	
	return NULL;
}


struct surface_t *skin_get_minigun_surface(uint32_t index)
{
	struct skin_t *skin = skin0;
		
	while(skin)
	{
		if(skin->index == index)
			return skin->minigun;
		
		skin = skin->next;
	}
	
	return NULL;
}


struct surface_t *skin_get_plasma_cannon_surface(uint32_t index)
{
	struct skin_t *skin = skin0;
		
	while(skin)
	{
		if(skin->index == index)
			return skin->plasma_cannon;
		
		skin = skin->next;
	}
	
	return NULL;
}
