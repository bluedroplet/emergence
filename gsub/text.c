/*
	Copyright (C) 1998-2002 Jonathan Brown
	
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

#include <stdint.h>

#include "../common/stringbuf.h"
#include "../common/vertex.h"
#include "gsub.h"

uint8_t charlengths[256];
struct surface_t *smallfont;


void init_text()
{
	smallfont = read_png_surface(PKGDATADIR "/em-client/smallfont.png");

	int i;

	for(i = 0; i != 256; i++)
	{
		switch(i)
		{
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


int blit_text(int x, int y, uint16_t colour, char *text)
{
	blit_source = smallfont;
	blit_colour = colour;

	int w = 0;

	while(*text)
	{
		blit_sourcex = ((int)*((uint8_t*)text)) << 3;
		blit_sourcey = 0;
		blit_destx = x + w;
		blit_desty = y;
		blit_height = 13;
		blit_width = charlengths[*((uint8_t*)text)];

		w += blit_width;

		blit_alpha_surface_rect();
		
		text++;
	}

	return w;
}
