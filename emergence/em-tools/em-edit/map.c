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


#ifdef LINUX
#define _GNU_SOURCE
#ifndef _REENTRANT
#define _REENTRANT
#endif
#endif

#include <stdint.h>
#include <unistd.h>

#include <gnome.h>
#include <zlib.h>

#include "../common/stringbuf.h"
#include "../common/rel2abs.h"
#include "../common/abs2rel.h"
#include "../gsub/gsub.h"
#include "nodes.h"
#include "conns.h"
#include "curves.h"
#include "points.h"
#include "lines.h"
#include "objects.h"
#include "bsp.h"
#include "tiles.h"
#include "fills.h"
#include "map.h"
#include "main.h"
#include "worker.h"
#include "glade.h"
#include "interface.h"

uint8_t map_active = 0;
uint8_t map_saved = 1;
uint8_t compiling = 0;

GtkWidget *compiling_dialog;


struct string_t *map_filename;
struct string_t *map_path;

void clear_map()		// always called when not working
{
	delete_all_nodes();
	delete_all_conns();
	delete_all_curves();
	delete_all_points();
	delete_all_fills();
	delete_all_lines();
	delete_all_tiles();
	delete_all_objects();

	map_saved = 1;
	string_clear(map_filename);
	string_clear(map_path);
	char *cwd =  get_current_dir_name();
	string_cat_text(map_path, cwd);
	free(cwd);
}


void compile()
{
	struct string_t *compile_filename = new_string();

	char *cc = map_filename->text;

	while(*cc && *cc != '.')
		string_cat_char(compile_filename, *cc++);

	string_cat_text(compile_filename, ".cm");

	gzFile file = gzopen(compile_filename->text, "wb9");

//	uint16_t format_id = EMERGENCE_COMPILEDFORMATID;
	
//	gzwrite(file, &format_id, 2);
	
	gzwrite_bsp_tree(file);
	gzwrite_objects_compiled(file);
	
	int num_tiles = count_tiles();
	gzwrite(file, &num_tiles, 4);

	struct tile_t *ctile = clean_tile0;

	while(ctile)
	{
		gzwrite(file, &ctile->x1, 4);
		gzwrite(file, &ctile->y2, 4);

		gzwrite_raw_surface(file, ctile->surface);

		ctile = ctile->next;
	}

	gzclose(file);
}


void init_map()
{
	map_filename = new_string();
	map_path = new_string();
}


void kill_map()
{
	delete_all_nodes();
	delete_all_conns();
	delete_all_curves();
	delete_all_points();
	delete_all_fills();
	delete_all_lines();
	delete_all_tiles();
	delete_all_objects();
	
	free_string(map_filename);
	free_string(map_path);
}


void set_map_path()
{
	string_clear(map_path);
	
	// count how many '/'s are in map_filename
	
	int slashes = 0;
	char *cc = map_filename->text;
	
	while(*cc)
	{
		if(*cc++ == '/')
			slashes++;
	}
	
	cc = map_filename->text;
	
	while(slashes)
	{
		string_cat_char(map_path, *cc);
		
		if(*cc++ == '/')
			slashes--;
	}
}


struct string_t *arb_rel2abs(char *path, char *base)
{
	size_t size = 100;
	char *buffer = (char*)malloc(size);
	if(!buffer)
		return NULL;
	
	if(rel2abs(path, base, buffer, size) == buffer)
	{
		struct string_t *string = malloc(sizeof(struct string_t));
		string->text = buffer;
		return string;
	}
	
	while(1)
	{
		size *= 2;
		
		char *buffer = (char*)realloc(buffer, size);
		if(!buffer)
			return NULL;
		
		if(rel2abs(path, base, buffer, size) == buffer)
		{
			struct string_t *string = malloc(sizeof(struct string_t));
			string->text = buffer;
			return string;
		}
		
		if(errno != ERANGE)
			return NULL;
	}
}


struct string_t *arb_abs2rel(char *path, char *base)
{
	size_t size = 100;
	char *buffer = (char*)malloc(size);
	if(!buffer)
		return NULL;
	
	if(abs2rel(path, base, buffer, size) == buffer)
	{
		struct string_t *string = malloc(sizeof(struct string_t));
		string->text = buffer;
		return string;
	}
	
	while(1)
	{
		size *= 2;
		
		char *buffer = (char*)realloc(buffer, size);
		if(!buffer)
			return NULL;
		
		if(abs2rel(path, base, buffer, size) == buffer)
		{
			struct string_t *string = malloc(sizeof(struct string_t));
			string->text = buffer;
			return string;
		}
			
		if(errno != ERANGE)
			return NULL;
	}
}


void on_specify_filename_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	;
}




int run_not_saved_dialog()
{
	GtkWidget *dialog = create_not_saved_dialog();
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	
	int ret = gtk_dialog_run(GTK_DIALOG(dialog));
	
	gtk_widget_destroy(GTK_WIDGET(dialog));
	
	return ret;
}	


void run_file_not_found_dialog(GtkWindow *parent)
{
	GtkWidget *dialog = create_file_not_found_dialog();
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	
	gtk_widget_show(dialog);
	gtk_main();
}	


void run_corrupt_file_dialog(GtkWindow *parent)
{
	GtkWidget *dialog = create_corrupt_file_dialog();
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	
	gtk_widget_show(dialog);
	gtk_main();
}	


void display_compile_dialog()
{
	compiling_dialog = create_map_compiling_dialog();
	
	gtk_window_set_transient_for(GTK_WINDOW(compiling_dialog), GTK_WINDOW(window));
	gtk_window_set_position(GTK_WINDOW(compiling_dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	
	gtk_widget_show_all(compiling_dialog);
}	


void destroy_compile_dialog()
{
	gtk_widget_destroy(compiling_dialog);
}	


int try_load_map(char *filename)
{
	gzFile gzfile = gzopen(filename, "rb");
	if(!gzfile)
		return -1;		// do something
	
	uint16_t fork_id, format_id;
	
	if(gzread(gzfile, &fork_id, 2) != 2)
		goto error;
	
	if(gzread(gzfile, &format_id, 2) != 2)
		goto error;
	
	if(fork_id != EMERGENCE_FORKID ||
		format_id != EMERGENCE_FORMATID)
		goto error;	// do something else
	
	clear_map();
	
	string_clear(map_filename);
	string_cat_text(map_filename, filename);
	set_map_path();
	
	if(!gzread_nodes(gzfile))
	{
		printf("a\n");
		goto error;
	}

	if(!gzread_conns(gzfile))
	{
		printf("b\n");
		goto error;
	}
	
	if(!gzread_curves(gzfile))
	{
		printf("ca\n");
		goto error;
	}

	if(!gzread_points(gzfile))
	{
		printf("d\n");
		goto error;
	}

	if(!gzread_fills(gzfile))
	{
		printf("e\n");
		goto error;
	}

	if(!gzread_lines(gzfile))
	{
		printf("f\n");
		goto error;
	}

	if(!gzread_objects(gzfile))
	{
		printf("g\n");
		goto error;
	}

	gzclose(gzfile);
	
	invalidate_bsp_tree();
	map_active = 1;

	return 0;

error:

	gzclose(gzfile);
	clear_map();
	map_active = 0;
	
	return -2;
}


void open_dialog_ok(GtkWidget *w, GtkFileSelection *fs)
{
	gtk_widget_set_sensitive(GTK_WIDGET(fs), 0);
	
	while(gtk_events_pending())
		gtk_main_iteration_do(0);
	
	stop_working();
	
	switch(try_load_map((char*)gtk_file_selection_get_filename(fs)))
	{
	case 0:
		view_all_map();
		break;
		
	case -1:
		run_file_not_found_dialog(GTK_WINDOW(fs));
		break;
	
	case -2:
		run_corrupt_file_dialog(GTK_WINDOW(fs));
		break;
	}
	
	gtk_widget_destroy(GTK_WIDGET(fs));
	
	while(gtk_events_pending())
		gtk_main_iteration_do(0);
	
	update_client_area();
	
	start_working();
}


void run_open_dialog()
{
	GtkWidget *file_selection = gtk_file_selection_new("Open Map");
	gtk_window_set_modal(GTK_WINDOW(file_selection), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(file_selection), GTK_WINDOW(window));
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(file_selection)->ok_button), "clicked", 
		G_CALLBACK(open_dialog_ok), file_selection);
	g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(file_selection)->cancel_button), "clicked", 
		G_CALLBACK(gtk_widget_destroy), G_OBJECT(file_selection));
	g_signal_connect(G_OBJECT(file_selection), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	gtk_widget_show(file_selection);
	
	gtk_main();
}


int map_save()
{
	gzFile gzfile = gzopen(map_filename->text, "wb9");
	if(!gzfile)
		return 0;
	
	uint16_t fork_id = EMERGENCE_FORKID, format_id = EMERGENCE_FORMATID;
	
	gzwrite(gzfile, &fork_id, 2);
	gzwrite(gzfile, &format_id, 2);
	
	gzwrite_nodes(gzfile);
	gzwrite_conns(gzfile);
	gzwrite_curves(gzfile);
	gzwrite_points(gzfile);
	gzwrite_fills(gzfile);
	gzwrite_lines(gzfile);
	gzwrite_objects(gzfile);

	gzclose(gzfile);

	map_saved = 1;

	return 1;
}


void save_dialog_ok(GtkFileSelection *file_selection)
{
	gtk_widget_set_sensitive(GTK_WIDGET(file_selection), 0);
	
	while(gtk_events_pending())
		gtk_main_iteration_do(0);
	
	stop_working();
	
	string_clear(map_filename);
	string_cat_text(map_filename, (char*)gtk_file_selection_get_filename(file_selection));
	set_map_path();
	
	map_save();
	
	start_working();

	g_object_set_data(G_OBJECT(file_selection), "retval", (gpointer)1);
	gtk_main_quit();
}


gboolean cancel(GtkFileSelection *file_selection)
{
	gtk_main_quit();
	return 1;
}


int run_save_dialog()	// returns 0 if user canceled
{
	GtkWidget *file_selection = gtk_file_selection_new("Save Map");
	gtk_window_set_modal(GTK_WINDOW(file_selection), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(file_selection), GTK_WINDOW(window));
	g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(file_selection)->ok_button), "clicked", 
		G_CALLBACK(save_dialog_ok), G_OBJECT(file_selection));
	g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(file_selection)->cancel_button), "clicked", 
		G_CALLBACK(cancel), G_OBJECT(file_selection));
	g_signal_connect_swapped(G_OBJECT(file_selection), "delete-event", 
		G_CALLBACK(cancel), G_OBJECT(file_selection));
	
	g_object_set_data(G_OBJECT(file_selection), "retval", (gpointer)0);

	gtk_widget_show(file_selection);
	
	gtk_main();
	
	int retval = (int)g_object_get_data(G_OBJECT(file_selection), "retval");
	
	gtk_widget_destroy(file_selection);
	
	return retval;
}


int run_save_first_dialog()
{
	GtkWidget *dialog = create_specify_filename_dialog();
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	
	int ret = gtk_dialog_run(GTK_DIALOG(dialog));
	
	gtk_widget_destroy(GTK_WIDGET(dialog));
	
	
	if(ret == GTK_RESPONSE_YES)
		ret = run_save_dialog();
	
	return ret;
}
