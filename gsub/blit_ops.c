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
#include <stdlib.h>
#include <string.h>

#include "gsub.h"

void alpha_surface_blit_mmx() __attribute__ ((cdecl));
void alpha_pixel_plot_x86()	__attribute__ ((cdecl));
void alpha_rect_draw_mmx() __attribute__ ((cdecl));
void surface_blit_mmx() __attribute__ ((cdecl));

// Blit variables

struct surface_t *blit_source;
int blit_sourcex, blit_sourcey;
struct surface_t *blit_dest;
int blit_destx, blit_desty;
int blit_width, blit_height;

uint16_t blit_colour;
uint8_t blit_alpha;


void rect_draw_c()
{
	uint8_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);
	
	int x, y = blit_height;

	while(y)
	{
		for(x = 0; x != blit_width; x++)
			((uint16_t*)dst)[x] = blit_colour;

		dst += blit_dest->pitch;
		y--;
	}
}


void alpha_pixel_plot_c()
{
	uint16_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);
	uint16_t oldcolour = *dst;
	uint8_t negalpha = ~blit_alpha;

	*dst = (vid_redalphalookup[((blit_colour & 0xf800) >> 3) | blit_alpha] + 
		vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
		(vid_greenalphalookup[((blit_colour & 0x7e0) << 3) | blit_alpha] +
		vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
		(vid_bluealphalookup[((blit_colour & 0x1f) << 8) | blit_alpha] + 
		vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
}


void alpha_rect_draw_c()
{
	uint8_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);

	uint16_t redalpha = vid_redalphalookup[((blit_colour & 0xf800) >> 3) | blit_alpha];
	uint16_t greenalpha = vid_greenalphalookup[((blit_colour & 0x7e0) << 3) | blit_alpha];
	uint16_t bluealpha = vid_bluealphalookup[((blit_colour & 0x1f) << 8) | blit_alpha];

	uint8_t negalpha = ~blit_alpha;

	int x, y = blit_height;
	
	while(y)
	{
		for(x = 0; x != blit_width; x++)
		{
			uint16_t oldcolour = ((uint16_t*)dst)[x];

			((uint16_t*)dst)[x] = (redalpha + vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(greenalpha + vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(bluealpha + vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		dst += blit_dest->pitch;
		y--;
	}
}


void surface_blit_c()
{
	uint8_t *src = get_pixel_addr(blit_source, blit_sourcex, blit_sourcey);
	uint8_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);

	int y = blit_height;

	while(y)
	{
		memcpy(dst, src, blit_width * 2);

		src += blit_source->pitch;
		dst += blit_dest->pitch;
		y--;
	}
}


void alpha_surface_blit_c()
{
	uint8_t *src = get_alpha_pixel_addr(blit_source, blit_sourcex, blit_sourcey);
	uint8_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);

	uint16_t red = (blit_colour & 0xf800) >> 3;
	uint16_t green = (blit_colour & 0x7e0) << 3;
	uint16_t blue = (blit_colour & 0x1f) << 8;

	int x, y = blit_height;

	while(y)
	{
		for(x = 0; x != blit_width; x++)
		{
			uint16_t oldcolour = ((uint16_t*)dst)[x];
			uint8_t alpha = src[x];
			uint8_t negalpha = ~alpha;

			((uint16_t*)dst)[x] = (vid_redalphalookup[red | alpha] + 
				vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(vid_greenalphalookup[green | alpha] +
				vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(vid_bluealphalookup[blue | alpha] + 
				vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		src += blit_source->alpha_pitch;
		dst += blit_dest->pitch;
		y--;
	}
}


void alpha_surface_alpha_blit_c()
{
	uint8_t *src = get_alpha_pixel_addr(blit_source, blit_sourcex, blit_sourcey);
	uint8_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);

	uint16_t red = (blit_colour & 0xf800) >> 3;
	uint16_t green = (blit_colour & 0x7e0) << 3;
	uint16_t blue = (blit_colour & 0x1f) << 8;

	int x, y = blit_height;

	while(y)
	{
		for(x = 0; x != blit_width; x++)
		{
			uint16_t oldcolour = ((uint16_t*)dst)[x];
			uint8_t alpha = ((int)src[x] * (int)blit_alpha) >> 8;
			uint8_t negalpha = ~alpha;

			((uint16_t*)dst)[x] = (vid_redalphalookup[red | alpha] + 
				vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(vid_greenalphalookup[green | alpha] +
				vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(vid_bluealphalookup[blue | alpha] + 
				vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		src += blit_source->alpha_pitch;
		dst += blit_dest->pitch;
		y--;
	}
}


void surface_alpha_blit_c()
{
	uint8_t *src = get_pixel_addr(blit_source, blit_sourcex, blit_sourcey);
	uint8_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);

	uint8_t negalpha = ~blit_alpha;

	int x, y = blit_height;

	while(y)
	{
		for(x = 0; x != blit_width; x++)
		{
			uint16_t oldcolour = ((uint16_t*)dst)[x];
			uint16_t blendcolour = ((uint16_t*)src)[x];

			((uint16_t*)dst) = (vid_redalphalookup[((blendcolour & 0xf800) >> 3) | blit_alpha] + 
				vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(vid_greenalphalookup[((blendcolour & 0x7e0) << 3) | blit_alpha] +
				vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(vid_bluealphalookup[((blendcolour & 0x1f) << 8) | blit_alpha] + 
				vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		src += blit_source->pitch;
		dst += blit_dest->pitch;
		y--;
	}
}


void surface_alpha_surface_blit_c()
{
	uint8_t *alphasrc = get_alpha_pixel_addr(blit_source, blit_sourcex, blit_sourcey);
	uint8_t *src = get_pixel_addr(blit_source, blit_sourcex, blit_sourcey);
	uint8_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);

	int x, y = blit_height;

	while(y)
	{
		for(x = 0; x != blit_width; x++)
		{
			uint8_t alpha = alphasrc[x];
			if(alpha == 0)
				continue;
			
			uint16_t oldcolour = ((uint16_t*)dst)[x];
			uint8_t negalpha = ~alpha;
			uint16_t blendcolour = ((uint16_t*)src)[x];

			((uint16_t*)dst)[x] = (vid_redalphalookup[((blendcolour & 0xf800) >> 3) | alpha] + 
				vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(vid_greenalphalookup[((blendcolour & 0x7e0) << 3) | alpha] +
				vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(vid_bluealphalookup[((blendcolour & 0x1f) << 8) | alpha] + 
				vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		alphasrc += blit_source->alpha_pitch;
		src += blit_source->pitch;
		dst += blit_dest->pitch;
		y--;
	}
}


void (*rect_draw)() = rect_draw_c;
void (*alpha_pixel_plot)() = alpha_pixel_plot_c;
void (*alpha_rect_draw)() = alpha_rect_draw_c;
void (*surface_blit)() = surface_blit_c;
void (*alpha_surface_blit)() = alpha_surface_blit_c;
void (*alpha_surface_alpha_blit)() = alpha_surface_alpha_blit_c;
void (*surface_alpha_blit)() = surface_alpha_blit_c;
void (*surface_alpha_surface_blit)() = surface_alpha_surface_blit_c;


int clip_blit_coords()
{
	if(blit_width == 0 || blit_height == 0)
		return 0;

	if((blit_destx + blit_width <= 0) || (blit_desty + blit_height <= 0) ||
		(blit_destx >= blit_dest->width) || (blit_desty >= blit_dest->height))
		return 0;

	if(blit_destx < 0)
	{
		blit_sourcex -= blit_destx;
		blit_width += blit_destx;
		blit_destx = 0;
	}

	if(blit_destx + blit_width > blit_dest->width)
		blit_width = blit_dest->width - blit_destx;

	if(blit_desty < 0)
	{
		blit_sourcey -= blit_desty;
		blit_height += blit_desty;
		blit_desty = 0;
	}

	if(blit_desty + blit_height > blit_dest->height)
		blit_height = blit_dest->height - blit_desty;

	return 1;
}


void plot_pixel()
{
	if(blit_destx < 0 || blit_destx >= blit_dest->width ||
		blit_desty < 0 || blit_desty >= blit_dest->height)
		return;

	uint16_t *dst = get_pixel_addr(blit_dest, blit_destx, blit_desty);
	*dst = blit_colour;
}


void draw_rect()
{
	if(!clip_blit_coords())
		return;

	rect_draw();
}


void plot_alpha_pixel()
{
	if(blit_destx < 0 || blit_destx >= blit_dest->width ||
		blit_desty < 0 || blit_desty >= blit_dest->height)
		return;

	alpha_pixel_plot();
}


void draw_alpha_rect()
{
	if(!clip_blit_coords())
		return;

	alpha_rect_draw();
}


void blit_surface()
{
	if(!blit_source)
		return;

	blit_sourcex = 0;
	blit_sourcey = 0;
	blit_width = blit_source->width;
	blit_height = blit_source->height;

	if(clip_blit_coords())
		surface_blit();
}


void blit_surface_rect()
{
	if(!blit_source)
		return;

	if(clip_blit_coords())
		surface_blit();
}


void blit_alpha_surface()
{
	if(!blit_source)
		return;

	blit_sourcex = 0;
	blit_sourcey = 0;
	blit_width = blit_source->width;
	blit_height = blit_source->height;
	
	if(clip_blit_coords())
		alpha_surface_blit();
}


void blit_alpha_surface_rect()
{
	if(!blit_source)
		return;

	if(clip_blit_coords())
		alpha_surface_blit();
}


void alpha_blit_alpha_surface()
{
	if(!blit_source)
		return;

	blit_sourcex = 0;
	blit_sourcey = 0;
	blit_width = blit_source->width;
	blit_height = blit_source->height;
	
	if(clip_blit_coords())
		alpha_surface_alpha_blit();
}


void alpha_blit_alpha_surface_rect()
{
	if(!blit_source)
		return;

	if(clip_blit_coords())
		alpha_surface_alpha_blit();
}


void alpha_blit_surface()
{
	if(!blit_source)
		return;

	blit_sourcex = 0;
	blit_sourcey = 0;
	blit_width = blit_source->width;
	blit_height = blit_source->height;

	if(clip_blit_coords())
		surface_alpha_blit();
}


void alpha_blit_surface_rect()
{
	if(!blit_source)
		return;

	if(clip_blit_coords())
		surface_alpha_blit();
}


void alpha_surface_blit_surface()
{
	if(!blit_source)
		return;

	blit_sourcex = 0;
	blit_sourcey = 0;
	blit_width = blit_source->width;
	blit_height = blit_source->height;
	
	if(clip_blit_coords())
		surface_alpha_surface_blit();
}


void alpha_surface_blit_surface_rect()
{
	if(!blit_source)
		return;

	if(clip_blit_coords())
		surface_alpha_surface_blit();
}
