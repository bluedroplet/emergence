/*
	Copyright (C) 1998-2004 Jonathan Brown
	
    This file is part of the gsub graphics library.
	
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source distribution.
	
	Jonathan Brown
	jbrown@emergence.uk.net
*/


#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "../common/prefix.h"

#include "../common/stringbuf.h"
#include "../common/resource.h"
#include "gsub.h"

uint8_t charlengths[256];
struct surface_t *smallfont;


void init_text()
{
	smallfont = read_png_surface(find_resource("smallfont.png"));

	int i;

	for(i = 0; i != 256; i++)
	{
		switch(i)
		{
		case ';':
		case ':':
		case '.':
		case 'i':
		case 'l':
		case 'j':
			charlengths[i] = 4;
			break;

			charlengths[i] = 5;
			break;

		case ' ':
		case '\\':
		case '/':
		case 'I':
		case 'r':
			charlengths[i] = 6;
			break;

		case 'f':
		case 'k':
		case 't':
			charlengths[i] = 7;
			break;
	
		default:
			charlengths[i] = 8;
			break;
		}
	}
}


int blit_text(int x, int y, uint8_t red, uint8_t green, uint8_t blue, 
	struct surface_t *dest, const char *fmt, ...)
{
	char *text;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&text, fmt, ap);
	va_end(ap);
	
	
	struct blit_params_t params;

	params.red = red;
	params.green = green;
	params.blue = blue;
	params.source = smallfont;
	params.dest = dest;
	
	int w = 0;
	char *c = text;
	
	while(*c)
	{
		params.source_x = ((int)*((uint8_t*)c)) << 3;
		params.source_y = 0;
		params.dest_x = x + w;
		params.dest_y = y;
		params.height = 13;
		params.width = charlengths[*((uint8_t*)c)];

		w += params.width;

		blit_partial_surface(&params);
		
		c++;
	}
	
	free(text);

	return w;
}


int blit_text_centered(int x, int y, uint8_t red, uint8_t green, uint8_t blue, 
	struct surface_t *dest, const char *fmt, ...)
{
	char *text;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&text, fmt, ap);
	va_end(ap);
	
	
	int w = 0;
	char *c = text;

	while(*c)
	{
		w += charlengths[*((uint8_t*)c)];
		c++;
	}
	
	x -= w / 2;

	
	struct blit_params_t params;

	params.red = red;
	params.green = green;
	params.blue = blue;
	params.source = smallfont;
	params.dest = dest;
	
	w = 0;
	c = text;

	while(*c)
	{
		params.source_x = ((int)*((uint8_t*)c)) << 3;
		params.source_y = 0;
		params.dest_x = x + w;
		params.dest_y = y;
		params.height = 13;
		params.width = charlengths[*((uint8_t*)c)];

		w += params.width;

		blit_partial_surface(&params);
		
		c++;
	}
	
	free(text);
	
	return w;
}


int blit_text_right_aligned(int x, int y, uint8_t red, uint8_t green, uint8_t blue, 
	struct surface_t *dest, const char *fmt, ...)
{
	char *text;
	
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&text, fmt, ap);
	va_end(ap);
	
	
	int w = 0;
	char *c = text;

	while(*c)
	{
		w += charlengths[*((uint8_t*)c)];
		c++;
	}
	
	x -= w;

	
	struct blit_params_t params;

	params.red = red;
	params.green = green;
	params.blue = blue;
	params.source = smallfont;
	params.dest = dest;
	
	w = 0;
	c = text;

	while(*c)
	{
		params.source_x = ((int)*((uint8_t*)c)) << 3;
		params.source_y = 0;
		params.dest_x = x + w;
		params.dest_y = y;
		params.height = 13;
		params.width = charlengths[*((uint8_t*)c)];

		w += params.width;

		blit_partial_surface(&params);
		
		c++;
	}
	
	free(text);
	
	return w;
}
