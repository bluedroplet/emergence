/* 
	Copyright (C) 1998-2002 Jonathan Brown

    This file is part of em-tools.

    em-tools is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    em-tools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with em-tools; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Jonathan Brown
	jbrown@emergence.uk.net
*/


#ifndef _INC_OBJECTS
#define _INC_OBJECTS


struct object_t
{
	uint8_t type;
	
	struct surface_t *texture_surface;
	struct surface_t *scaled_texture_surface;
	
	float x, y;
	
	union
	{
		struct
		{
			int plasmas;
			float angle;
			int respawn_delay;
			
		} plasma_cannon_data;
	
		struct
		{
			int bullets;
			float angle;
			int respawn_delay;
			
		} minigun_data;
	
		struct
		{
			int rockets;
			float angle;
			int respawn_delay;
			
		} rocket_launcher_data;
	
		struct
		{
			int quantity;
			float angle;
			int respawn_delay;
			
		} rails_data;
	
		struct
		{
			int energy;
			float angle;
			int respawn_delay;
			
		} shield_energy_data;

		struct
		{
			int non_default_texture;
			struct string_t *texture_filename;
			struct surface_t *texture_pre_surface;
			float width, height;
			float angle;
			int teleport_only;
			int index;
			
		} spawn_point_data;
	
		struct
		{
			int non_default_texture;
			struct string_t *texture_filename;
			struct surface_t *texture_pre_surface;
			float width, height;
			float angle;
			float activation_width;
			float boost;
			
		} speedup_ramp_data;
	
		struct
		{
			int non_default_texture;
			struct string_t *texture_filename;
			struct surface_t *texture_pre_surface;
			float width, height;
			float angle;
			float radius;
			int sparkles;
			int spawn_point_index;
			int rotation_type;
			float rotation_angle;
			int divider;
			float divider_angle;
			
		} teleporter_data;
	
		struct
		{
			int non_default_texture;
			struct string_t *texture_filename;
			struct surface_t *texture_pre_surface;
			float width, height;
			float angle;
			float strength;
			int enclosed;
			
		} gravity_well_data;
	};
	
	struct object_t *next;
};


struct object_pointer_t
{
	struct object_t *object;
	struct object_pointer_t *next;
};


#define OBJECTTYPE_PLASMACANNON		0
#define OBJECTTYPE_MINIGUN			1
#define OBJECTTYPE_ROCKETLAUNCHER	2
#define OBJECTTYPE_RAILS			3
#define OBJECTTYPE_SHIELDENERGY		4
#define OBJECTTYPE_SPAWNPOINT		5
#define OBJECTTYPE_SPEEDUPRAMP		6
#define OBJECTTYPE_TELEPORTER 		7
#define OBJECTTYPE_GRAVITYWELL		8

#define TELEPORTER_ROTATION_TYPE_ABS	0
#define TELEPORTER_ROTATION_TYPE_REL	1

extern struct object_t *object0;



void init_objects();
void kill_objects();
void draw_objects();
void insert_object(int type, float x, float y);
void invalidate_all_scaled_objects();
void delete_object(struct object_t *object);
void delete_all_objects();

int check_for_unresampled_objects();
void resample_object();
void finished_resampling_object();

int check_for_unscaled_objects();
void scale_object();
void finished_scaling_object();

struct object_t *get_object(int x, int y, int *xoffset, int *yoffset);

int add_object_pointer(struct object_pointer_t **objectp0, struct object_t *object);
void remove_object_pointer(struct object_pointer_t **objectp0, struct object_t *object);
int object_in_object_pointer_list(struct object_pointer_t *objectp0, struct object_t *object);

int count_object_floating_images();


#ifdef ZLIB_H
void gzwrite_objects(gzFile file);
void gzwrite_objects_compiled(gzFile file);
int gzread_objects(gzFile file);
void gzwrite_object_floating_images(gzFile file);
#endif

void run_object_menu(struct object_t *object);


#endif	// _INC_OBJECTS
