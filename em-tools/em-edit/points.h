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

#ifndef _INC_POINTS
#define _INC_POINTS

struct point_t
{
	uint32_t index;	// for saving

	struct curve_t *curve;

	double pos;		// 0..1
	
	double x, y;
	double deltax, deltay;
	double left_width, right_width;
	
	struct conn_t *conn;
	double t;
	int t_index;
	
	struct point_t *next;
};


struct point_pointer_t
{
	struct point_t *point;

	struct point_pointer_t *next;
};

void insert_point(struct curve_t *curve, double x, double y);
void remove_point(struct point_t *point);
int add_point_pointer(struct point_pointer_t **pointp0, struct point_t *point);
void update_point_positions();
void move_point(struct point_t *point, double x, double y);
struct point_t *get_point(int x, int y, int *xoffset, int *yoffset);

uint32_t count_point_pointers(struct point_pointer_t *pointp0);
uint32_t count_points();
	
#if defined _ZLIB_H
void gzwrite_point_pointer_list(gzFile file, struct point_pointer_t *pointp0);
void gzwrite_points(gzFile file);
int gzread_points(gzFile file);
#endif

struct point_t *get_point_from_index(uint32_t index);
void draw_points();

void delete_all_points();
void run_point_menu(struct point_t *point);

extern struct point_t *point0;

void init_points();
void kill_points();

#endif	// _INC_POINTS
