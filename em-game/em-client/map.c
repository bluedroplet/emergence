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

struct string_t *map_filename;


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
		
		struct surface_t *surface = resize(tile.surface, tile.surface->width * vid_width / 1600, 
			tile.surface->height * vid_height / 1200, NULL);

		free_surface(tile.surface);
		tile.surface = surface;
		

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


void load_map(char *name)
{
	printf(name);
	printf("\n");
	gzFile gzfile = gzopen(name, "rb");
	if(!gzfile)
	{
		console_print("Could not load map: %s\n", name);
		return 1;
	}
	
	console_print("Loading BSP Tree\n");
	load_bsp_tree(gzfile);
	bypass_objects(gzfile);
	console_print("Loading Map Tiles\n");
	load_map_tiles(gzfile);
	
	gzclose(gzfile);
}


int game_process_load_map(struct buffer_t *stream)
{
	struct string_t *map_name = buffer_read_string(stream);
	
	map_filename = new_string_string(emergence_home_dir);
	string_cat_text(map_filename, "/maps/");	
	string_cat_string(map_filename, map_name);
	string_cat_text(map_filename, ".cmap");


	load_map(map_filename->text);
	free_string(map_name);
	
	console_print("Map loaded ok.\n");
	
	return 1;
}


void reload_map()
{
	if(map_filename)
		load_map(map_filename->text);
}

/*
int load_map_tiles(char *filename)
{
	FILE *file = fopen(filename, "rb");
	if(!file)
		goto error;
	
//	load_bsp_tree(file);
	
	int i;
	int num_tiles;

	if(fread(&num_tiles, 4, 1, file) != 1)
		goto error;

	for(i = 0; i < num_tiles; i++)
	{
		struct tile_t tile;

		if(fread(&tile.x, 4, 1, file) != 1)
			goto error;

		if(fread(&tile.y, 4, 1, file) != 1)
			goto error;

	//	tile.surface = read_raw_surface(file);

		LL_ADD(struct tile_t, &tile0, &tile);
	}

	fclose(file);
	return 1;

error:

	if(file)
		fclose(file);

	return 0;
}

*/
int count_tiles()
{
	struct tile_t *tile = tile0;
	int count = 0;

	while(tile)
	{
		count++;
		tile = tile->next;
	}

	return count;
}


int generate_scaled_map(char *name)
{
	struct string_t *filename = new_string_text(name);
	string_cat_text(filename, ".cmap");
	if(!load_map_tiles(filename->text))
		return 0;

	struct tile_t *tile = tile0;
	
	while(tile)
	{
		struct surface_t *surface = resize(tile->surface, tile->surface->width * vid_width / 1600, 
			tile->surface->height * vid_height / 1200, NULL);

		free_surface(tile->surface);
		tile->surface = surface;

		tile = tile->next;
	}

	string_clear(filename);
	string_cat_text(filename, name);
	string_cat_char(filename, '.');
	string_cat_int(filename, vid_width);
	string_cat_text(filename, ".cmap");

	FILE *file = fopen(filename->text, "wb");
	free_string(filename);
	
	write_bsp_tree(file);

	int num_tiles = count_tiles();
	fwrite(&num_tiles, 4, 1, file);

	struct tile_t *ctile = tile0;

	while(ctile)
	{
		fwrite(&ctile->x, 4, 1, file);
		fwrite(&ctile->y, 4, 1, file);

	//	write_raw_surface(ctile->surface, file);

		ctile = ctile->next;
	}

	fclose(file);

	return 1;
}

/*
int load_map(char *name)
{
	LL_REMOVE_ALL(struct tile_t, &tile0);

	struct string_t *filename = new_string_text(name);

	switch(vid_width)
	{
	case 640:
		string_cat_text(filename, ".640.cmap");
		if(!load_map_tiles(filename->text))
		{
			if(!generate_scaled_map(name))
				return 0;
		}

		break;

	case 800:
		string_cat_text(filename, ".800.cmap");
		if(!load_map_tiles(filename->text))
		{
			if(!generate_scaled_map(name))
				return 0;
		}

		break;

	case 1024:
		string_cat_text(filename, ".1024.cmap");
		if(!load_map_tiles(filename->text))
		{
			if(!generate_scaled_map(name))
				return 0;
		}

		break;

	case 1152:
		string_cat_text(filename, ".1152.cmap");
		if(!load_map_tiles(filename->text))
		{
			if(!generate_scaled_map(name))
				return 0;
		}

		break;

	case 1280:
		string_cat_text(filename, ".1280.cmap");
		if(!load_map_tiles(filename->text))
		{
			if(!generate_scaled_map(name))
				return 0;
		}

		break;

	case 1600:
		string_cat_text(filename, ".cmap");
		if(!load_map_tiles(filename->text))
			return 0;
		break;
	}

	free_string(filename);

	return 1;
}
*/

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
