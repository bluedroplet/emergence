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
