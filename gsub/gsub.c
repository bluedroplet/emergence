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
#ifndef _REENTRANT
#define _REENTRANT
#endif
#endif

#include <stdint.h>
#include <malloc.h>
#include <memory.h>

#include "../common/stringbuf.h"
#include "../common/vertex.h"
#include "gsub.h"


uint8_t *vid_alphalookup = NULL;
uint16_t *vid_graylookup = NULL;
uint16_t *vid_redalphalookup = NULL;
uint16_t *vid_greenalphalookup = NULL;
uint16_t *vid_bluealphalookup = NULL;


int (*gsub_callback)();

/*
void clear_backbuffer()
{
//	printf("%u\n", (uint32_t)vid_backbuffer % 8);
	
	bb_clear_mmx(vid_backbuffer, (vid_pitch * vid_height) / 32, 0);
	//((vid_pitch * vid_height) % 16) / 2);
	
	if(vid_pitch == vid_width)
	{
		memset(vid_backbuffer, 0, vid_width * vid_height * 2);
	}
	else
	{
		uint16_t *dst = vid_backbuffer;
		int y, bytes_width = vid_width * 2;

		for(y = 0; y < vid_height; y++)
		{
			memset(dst, 0, bytes_width);
			dst += vid_pitch;
		}
	}

}
*/

void make_lookup_tables()
{
	vid_alphalookup = malloc(65536);
	vid_graylookup = malloc(512);
	vid_redalphalookup = malloc(16384);
	vid_greenalphalookup = malloc(32768);
	vid_bluealphalookup = malloc(16384);

	
	int a, b, r, d, q;
	uint8_t *dst8 = vid_alphalookup;

	for(a = 0; a < 256; a++)
	{
		for(b = 0; b < 256; b++)
		{
			r = a * b;
			d = r / 255;
			q = r % 255;

			if(q < 128)
				*dst8++ = d;
			else
				*dst8++ = d + 1;
		}
	}

	
	uint16_t o;
	uint16_t *dst16 = vid_graylookup;
	
	for(a = 0; a < 256; a++)
	{
		r = a * 31;
		d = r / 255;
		q = r % 255;

		if(q < 128)
			o = d | (d << 11);
		else
			o = (d + 1) | ((d + 1) << 11);

		r = a * 63;
		d = r / 255;
		q = r % 255;

		if(q < 128)
			o |= d << 5;
		else
			o |= (d + 1) << 5;

		*dst16++ = o;	// bollocks
	}

	
	dst16 = vid_redalphalookup;
	
	for(a = 0; a < 32; a++)
	{
		for(b = 0; b < 256; b++)
		{
			r = a * b;
			d = r / 255;
			q = r % 255;

			if(q < 128)
				*dst16++ = d << 11;
			else
				*dst16++ = (d + 1) << 11;
		}
	}

	
	dst16 = vid_greenalphalookup;
	
	for(a = 0; a < 64; a++)
	{
		for(b = 0; b < 256; b++)
		{
			r = a * b;
			d = r / 255;
			q = r % 255;

			if(q < 128)
				*dst16++ = d << 5;
			else
				*dst16++ = (d + 1) << 5;
		}
	}

	
	dst16 = vid_bluealphalookup;
	
	for(a = 0; a < 32; a++)
	{
		for(b = 0; b < 256; b++)
		{
			r = a * b;
			d = r / 255;
			q = r % 255;

			if(q < 128)
				*dst16++ = d;
			else
				*dst16++ = d + 1;
		}
	}
}


uint16_t convert_24bit_to_16bit(uint8_t red, uint8_t green, uint8_t blue)
{
	int r = red * 31;
	int d = r / 255;
	int q = r % 255;

	uint16_t o;

	if(q < 128)
		o = d << 11;
	else
		o = (d + 1) << 11;

	r = green * 63;
	d = r / 255;
	q = r % 255;

	if(q < 128)
		o |= d << 5;
	else
		o |= (d + 1) << 5;

	r = blue * 31;
	d = r / 255;
	q = r % 255;

	if(q < 128)
		o |= d;
	else
		o |= d + 1;

	return o;
}


void init_gsub()
{
	make_lookup_tables();
	init_text();
}


void kill_gsub()
{
	free(vid_alphalookup);
	free(vid_graylookup);
	free(vid_redalphalookup);
	free(vid_greenalphalookup);
	free(vid_bluealphalookup);
}
