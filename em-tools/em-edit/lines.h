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


#ifndef _INC_LINES
#define _INC_LINES


struct line_pointer_t
{
	struct line_t *line;
	struct line_pointer_t *next;
};


struct line_t
{
	uint8_t status;	// door and/or switch may be active

	struct point_t *point1, *point2;
	
	uint8_t door_red, door_green, door_blue, door_alpha;
	float door_width;
	float door_energy;
	uint8_t door_initial_state;
	uint16_t door_open_timeout;
	uint16_t door_close_timeout;
	uint16_t door_index;
	
	uint8_t switch_red, switch_green, switch_blue, switch_alpha;
	float switch_width;

	struct line_pointer_t *switch_in_door_close_list, 
		*switch_in_door_open_list, *switch_in_door_invert_list;
	
	struct line_pointer_t *switch_out_door_close_list, 
		*switch_out_door_open_list, *switch_out_door_invert_list;
	
	struct line_t *next;
};


#define LINE_STATUS_DOOR 1
#define LINE_STATUS_SWITCH 2

#define DOOR_INITIAL_STATE_OPEN 1
#define DOOR_INITIAL_STATE_CLOSED 2


void insert_line(struct point_t *point1, struct point_t *point2);
void insert_follow_curve_line(struct point_t *point1, struct point_t *point2);
struct line_t *get_line(float x, float y);
void draw_some_lines();
void draw_all_lines();
void draw_switches_and_doors();
void delete_line(struct line_t *line);
void run_door_switch_properties_dialog(void *menu, struct line_t *line);

uint32_t count_lines();
uint32_t count_line_pointers(struct line_pointer_t *linep0);

#if defined ZLIB_H
void gzwrite_line_pointer_list(gzFile file, struct line_pointer_t *linep0);
void gzwrite_lines(gzFile file);
int gzread_lines(gzFile file);
#endif

void run_line_menu(struct line_t *line);


void delete_all_lines();

#endif	// _INC_LINES
