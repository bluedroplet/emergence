/* 
	Copyright (C) 1998-2002 Jonathan Brown

    This file is part of em-edit.

    em-edit is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    em-edit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with em-edit; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Jonathan Brown
	jbrown@emergence.uk.net
*/



#ifndef _INC_INTERFACE
#define _INC_INTERFACE

extern double zoom;

extern int mouse_screenx, mouse_screeny;
extern double mouse_worldx, mouse_worldy;

extern int left_button_down_screenx, left_button_down_screeny;
extern double left_button_down_worldx, left_button_down_worldy;

extern int right_button_down_screenx, right_button_down_screeny;
extern double right_button_down_worldx, right_button_down_worldy;

extern int right_button_down_rootx, right_button_down_rooty;


void world_to_screen(double worldx, double worldy, int *screenx, int *screeny);
void screen_to_world(int screenx, int screeny, double *worldx, double *worldy);

void draw_world_clipped_line(double x1, double y1, double x2, double y2);


#define VIEW_GRID				0x0001
#define VIEW_OBJECTS			0x0002
#define VIEW_TILES				0x0004
#define VIEW_BOXES				0x0008
#define VIEW_BSP_TREE			0x0010
#define VIEW_OUTLINES			0x0020
#define VIEW_SWITCHES_AND_DOORS	0x0040
#define VIEW_NODES				0x0080

extern uint16_t view_state;

void invert_view_grid();
void invert_view_objects();
void invert_view_tiles();
void invert_view_boxes();
void invert_view_bsp_tree();
void invert_view_outlines();
void invert_view_switches_and_doors();
void invert_view_nodes();


#define SATS_VECT	0
#define SATS_WIDTH	1

extern uint8_t view_sats_mode;


void view_vect_sats();
void view_width_sats();


void draw_all();
void key_pressed(uint32_t key);
void key_released(uint32_t key);
void left_button_down(int x, int y);
void left_button_up(int x, int y);
void right_button_down(int x, int y, int rootx, int rooty);
void right_button_up(int x, int y);
void mouse_wheel_up(int x, int y);
void mouse_wheel_down(int x, int y);
int mouse_move(int x, int y);
void window_size_changed();
void scale_map_to_window();	// called while not working

void view_all_map();


#ifdef _INC_NODES
void node_deleted(struct node_t *node);
extern struct node_pointer_t *selected_node0;
void connect_straight(struct node_t *node);
void connect_conic(struct node_t *node);
void connect_bezier(struct node_t *node);
#endif

#ifdef	_INC_OBJECTS
void object_deleted(struct object_t *object);
extern struct object_pointer_t *selected_object0;
#endif

struct surface_t *s_select;

#ifdef	_INC_POINTS
void join_point();
void define_fill(struct point_t *point);
void finished_defining_fill();
#endif

void quit();

void action_new_map();
void action_clear_map();

void init_interface();
void kill_interface();

#endif	// _INC_INTERFACE
