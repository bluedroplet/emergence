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


#ifndef _INC_BSP
#define _INC_BSP

void invalidate_bsp_tree();
void generate_bsp_tree();
void generate_ui_bsp_tree();
#ifdef _ZLIB_H
void gzwrite_bsp_tree(gzFile file);
#endif
void draw_bsp_tree();
void kill_bsp_tree();

extern int generate_bsp, generate_ui_bsp;
struct curve_t *get_curve_bsp(double x, double y);
struct node_t *get_node_bsp(double x, double y);
struct fill_t *get_fill_bsp(double x, double y);
void finished_generating_bsp_tree();
void finished_generating_ui_bsp_tree();
void clear_bsp_trees();
void more_bsp();
void less_bsp();

#endif	// _INC_BSP
