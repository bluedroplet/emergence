/* 
	Copyright (C) 1998-2002 Jonathan Brown

    This file is part of em-skin.

    em-rotate is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    em-rotate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with em-rotate; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Jonathan Brown
	jbrown@emergence.uk.net
*/


#ifdef LINUX
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdint.h>
#include <error.h>
#include <argp.h>

#include <zlib.h>

#include "../gsub/gsub.h"
#include "../common/stringbuf.h"
#include "../common/user.h"



void skin(char *skin)
{
	struct string_t *filename = new_string_text("%s.skin", skin);
	
	gzFile gzfile = gzopen(filename->text, "wb9");
	if(!gzfile)
		return;
	
	free_string(filename);
	struct surface_t *surface;
		
	filename = new_string_text("%s/craft.png", skin);
	surface	= read_png_surface_as_24bitalpha8bit(filename->text);
	gzwrite_raw_surface(gzfile, surface);
	free_surface(surface);
	free_string(filename);
	
	filename = new_string_text("%s/rocket-launcher.png", skin);
	surface	= read_png_surface_as_24bitalpha8bit(filename->text);
	gzwrite_raw_surface(gzfile, surface);
	free_surface(surface);
	free_string(filename);
	
	filename = new_string_text("%s/minigun.png", skin);
	surface	= read_png_surface_as_24bitalpha8bit(filename->text);
	gzwrite_raw_surface(gzfile, surface);
	free_surface(surface);
	free_string(filename);
	
	filename = new_string_text("%s/plasma-cannon.png", skin);
	surface	= read_png_surface_as_24bitalpha8bit(filename->text);
	gzwrite_raw_surface(gzfile, surface);
	free_surface(surface);
	free_string(filename);

	gzclose(gzfile);
}
	

const char *argp_program_version = "em-skin " VERSION;
const char *argp_program_bug_address = "<jbrown@emergence.uk.net>";
static char doc[] = "Generates Emergence skin files from exploded directories";
static char args_doc[] = "directories";

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	switch (key)
	{
	case ARGP_KEY_ARG:

		skin(arg);
		break;

	case ARGP_KEY_END:
		if (state->arg_num < 1)
		/* Not enough arguments. */
			argp_usage (state);

		break;


	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}


static struct argp argp = { NULL, parse_opt, args_doc, doc };

int main (int argc, char **argv)
{
	init_user();
	argp_parse(&argp, argc, argv, 0, 0, NULL);
	exit(0);
}
