#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>

#include <zlib.h>

#include "../common/types.h"
#include "shared/cvar.h"
#include "shared/bsp.h"
#include "shared/objects.h"
#include "shared/sgame.h"
#include "../common/llist.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../gsub/gsub.h"
#include "render.h"
#include "game.h"
#include "bsp.h"
#include "console.h"
#include "entry.h"

struct tile_t
{
	int x, y;	// unzoomed pixels from south west origin always multiple of 50
	struct surface_t *surface;
	struct tile_t *next;

} *tile0 = NULL;


int r_DrawBSPTree = 0;

struct string_t *map_name;


int load_map_tiles(gzFile gzfile)
{
	LL_REMOVE_ALL(struct tile_t, &tile0);
	
	int num_tiles;

	if(gzread(gzfile, &num_tiles, 4) != 4)
		goto error;
	
	while(num_tiles--)
	{
		struct tile_t tile;

		if(gzread(gzfile, &tile.x, 4) != 4)
			goto error;

		if(gzread(gzfile, &tile.y, 4) != 4)
			goto error;

		tile.surface = gzread_raw_surface(gzfile);
		
		LL_ADD(struct tile_t, &tile0, &tile);
	}

	return 1;

error:

	return 0;
}


int generate_and_write_scaled_tiles(gzFile gzfile, gzFile gzfileout)
{
	LL_REMOVE_ALL(struct tile_t, &tile0);
	
	int num_tiles;

	if(gzread(gzfile, &num_tiles, 4) != 4)
		goto error;
	
	if(gzwrite(gzfileout, &num_tiles, 4) != 4)
		goto error;
	
	while(num_tiles--)
	{
		struct tile_t tile;

		if(gzread(gzfile, &tile.x, 4) != 4)
			goto error;

		if(gzwrite(gzfileout, &tile.x, 4) != 4)
			goto error;

		if(gzread(gzfile, &tile.y, 4) != 4)
			goto error;

		if(gzwrite(gzfileout, &tile.y, 4) != 4)
			goto error;

		tile.surface = gzread_raw_surface(gzfile);
		
		struct surface_t *surface = resize(tile.surface, tile.surface->width * vid_width / 1600, 
			tile.surface->height * vid_height / 1200, NULL);

		free_surface(tile.surface);
		tile.surface = surface;
		
		gzwrite_raw_surface(gzfileout, tile.surface);

		LL_ADD(struct tile_t, &tile0, &tile);
	}

	return 1;

error:

	return 0;
}


void bypass_objects(gzFile gzfile)
{
	uint32_t num_objects;
	
	if(gzread(gzfile, &num_objects, 4) != 4)
		return;
	
	int o;
	for(o = 0; o < num_objects; o++)
	{
		uint8_t type;
	
		if(gzread(gzfile, &type, 1) != 1)
			return;
		
		
		switch(type)
		{
		case OBJECTTYPE_PLASMACANNON:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 16, SEEK_CUR);
			break;
		
		case OBJECTTYPE_MINIGUN:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 16, SEEK_CUR);
			break;
		
		case OBJECTTYPE_ROCKETLAUNCHER:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 16, SEEK_CUR);
			break;
		
		case OBJECTTYPE_RAILS:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 16, SEEK_CUR);
			break;
		
		case OBJECTTYPE_SHIELDENERGY:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 16, SEEK_CUR);
			break;
		
		case OBJECTTYPE_SPAWNPOINT:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 16, SEEK_CUR);
			break;
			
		case OBJECTTYPE_SPEEDUPRAMP:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 24, SEEK_CUR);
			break;
			
		case OBJECTTYPE_TELEPORTER:
			gzseek(gzfile, 16, SEEK_CUR);
			gzseek(gzfile, 12, SEEK_CUR);
			break;
			
		case OBJECTTYPE_GRAVITYWELL:
			read_gravity_well(gzfile);
			break;
		}
	}
}


int load_map(char *map_name)
{
	console_print(map_name);
	console_print("\n");
	
	struct string_t *map_filename = new_string_string(emergence_home_dir);
	string_cat_text(map_filename, "/maps/");
	string_cat_text(map_filename, map_name);
	string_cat_text(map_filename, ".cmap");

	gzFile gzfile = gzopen(map_filename->text, "rb");
	if(!gzfile)
	{
		console_print("Could not load map: %s\n", map_name);
		return 1;
	}
	
	if(vid_width == 1600)
	{
		console_print("Loading BSP tree\n");
		load_bsp_tree(gzfile);
		bypass_objects(gzfile);
		console_print("Loading map tiles\n");
		load_map_tiles(gzfile);
	}
	else
	{
		struct string_t *cached_filename = new_string_string(map_filename);
		string_cat_text(cached_filename, "%s%u", ".cache", vid_width);
		
		console_print("Loading BSP tree\n");
		load_bsp_tree(gzfile);
		
		gzFile gzcachedfile = gzopen(cached_filename->text, "rb");
		if(gzcachedfile)
		{
			console_print("Loading cached scaled map tiles\n");
			
			load_map_tiles(gzcachedfile);
		}
		else
		{
			gzFile gzcachedfile = gzopen(cached_filename->text, "w9b");
			if(!gzcachedfile)
				return 0;
			
			console_print("Scaling and caching map tiles\n");
			
			bypass_objects(gzfile);
			
			if(!generate_and_write_scaled_tiles(gzfile, gzcachedfile))
				return 0;
			
			gzclose(gzcachedfile);
		}
		
		free_string(cached_filename);
	}
	
	gzclose(gzfile);
	
	free_string(map_filename);

	console_print("Map loaded ok.\n");
	
	return 1;
}


int game_process_load_map(struct buffer_t *stream)
{
	map_name = buffer_read_string(stream);
	
	return load_map(map_name->text);
}


void reload_map()
{
	if(map_name)
		load_map(map_name->text);
}


void render_map()
{
	struct tile_t *tile = tile0;
		
	struct blit_params_t params;
		
	params.dest = s_backbuffer;

	while(tile)
	{
		int x, y;

		world_to_screen((double)tile->x, (double)tile->y, &x, &y);


		params.source = tile->surface;
	
		params.dest_x = x;
		params.dest_y = y;
	
		blit_surface(&params);

		tile = tile->next;
	}
	
	if(r_DrawBSPTree)
		draw_bsp_tree();
}


void init_map_cvars()
{
	create_cvar_int("r_DrawBSPTree", &r_DrawBSPTree, 0);
}
