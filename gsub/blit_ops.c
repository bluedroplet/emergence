/*
	Copyright (C) 1998-2003 Jonathan Brown
	
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



void rect_draw_c(struct blit_params_t *params)
{
	uint8_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);
	
	int x, y = params->height;

	while(y)
	{
		for(x = 0; x != params->width; x++)
			((uint16_t*)dst)[x] = params->colour16;

		dst += params->dest->pitch;
		y--;
	}
}


void alpha_pixel_plot_c(struct blit_params_t *params)
{
	uint16_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);
	uint16_t oldcolour = *dst;
	uint8_t negalpha = ~params->alpha;

	*dst = (vid_redalphalookup[((params->colour16 & 0xf800) >> 3) | params->alpha] + 
		vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
		(vid_greenalphalookup[((params->colour16 & 0x7e0) << 3) | params->alpha] +
		vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
		(vid_bluealphalookup[((params->colour16 & 0x1f) << 8) | params->alpha] + 
		vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
}


void alpha_rect_draw_c(struct blit_params_t *params)
{
	uint8_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);

	uint16_t redalpha = vid_redalphalookup[((params->colour16 & 0xf800) >> 3) | params->alpha];
	uint16_t greenalpha = vid_greenalphalookup[((params->colour16 & 0x7e0) << 3) | params->alpha];
	uint16_t bluealpha = vid_bluealphalookup[((params->colour16 & 0x1f) << 8) | params->alpha];

	uint8_t negalpha = ~params->alpha;

	int x, y = params->height;
	
	while(y)
	{
		for(x = 0; x != params->width; x++)
		{
			uint16_t oldcolour = ((uint16_t*)dst)[x];

			((uint16_t*)dst)[x] = (redalpha + vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(greenalpha + vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(bluealpha + vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		dst += params->dest->pitch;
		y--;
	}
}


void surface_blit_c(struct blit_params_t *params)
{
	uint8_t *src = get_pixel_addr(params->source, params->source_x, params->source_y);
	uint8_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);

	int y = params->height;

	while(y)
	{
		memcpy(dst, src, params->width * 2);

		src += params->source->pitch;
		dst += params->dest->pitch;
		y--;
	}
}


void alpha_surface_blit_c(struct blit_params_t *params)
{
	uint8_t *src = get_alpha_pixel_addr(params->source, params->source_x, params->source_y);
	uint8_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);

	uint16_t red = (params->colour16 & 0xf800) >> 3;
	uint16_t green = (params->colour16 & 0x7e0) << 3;
	uint16_t blue = (params->colour16 & 0x1f) << 8;

	int x, y = params->height;

	while(y)
	{
		for(x = 0; x != params->width; x++)
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

		src += params->source->alpha_pitch;
		dst += params->dest->pitch;
		y--;
	}
}


void alpha_surface_alpha_blit_c(struct blit_params_t *params)
{
	uint8_t *src = get_alpha_pixel_addr(params->source, params->source_x, params->source_y);
	uint8_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);

	uint16_t red = (params->colour16 & 0xf800) >> 3;
	uint16_t green = (params->colour16 & 0x7e0) << 3;
	uint16_t blue = (params->colour16 & 0x1f) << 8;

	int x, y = params->height;

	while(y)
	{
		for(x = 0; x != params->width; x++)
		{
			uint16_t oldcolour = ((uint16_t*)dst)[x];
			uint8_t alpha = ((int)src[x] * (int)params->alpha) >> 8;
			uint8_t negalpha = ~alpha;

			((uint16_t*)dst)[x] = (vid_redalphalookup[red | alpha] + 
				vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(vid_greenalphalookup[green | alpha] +
				vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(vid_bluealphalookup[blue | alpha] + 
				vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		src += params->source->alpha_pitch;
		dst += params->dest->pitch;
		y--;
	}
}


void surface_alpha_blit_c(struct blit_params_t *params)
{
	uint8_t *src = get_pixel_addr(params->source, params->source_x, params->source_y);
	uint8_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);

	uint8_t negalpha = ~params->alpha;

	int x, y = params->height;

	while(y)
	{
		for(x = 0; x != params->width; x++)
		{
			uint16_t oldcolour = ((uint16_t*)dst)[x];
			uint16_t blendcolour = ((uint16_t*)src)[x];

			((uint16_t*)dst) = (vid_redalphalookup[((blendcolour & 0xf800) >> 3) | params->alpha] + 
				vid_redalphalookup[((oldcolour & 0xf800) >> 3) | negalpha]) |
				(vid_greenalphalookup[((blendcolour & 0x7e0) << 3) | params->alpha] +
				vid_greenalphalookup[((oldcolour & 0x7e0) << 3) | negalpha]) |
				(vid_bluealphalookup[((blendcolour & 0x1f) << 8) | params->alpha] + 
				vid_bluealphalookup[((oldcolour & 0x1f) << 8) | negalpha]);
		}

		src += params->source->pitch;
		dst += params->dest->pitch;
		y--;
	}
}


void surface_alpha_surface_blit_c(struct blit_params_t *params)
{
	uint8_t *alphasrc = get_alpha_pixel_addr(params->source, params->source_x, params->source_y);
	uint8_t *src = get_pixel_addr(params->source, params->source_x, params->source_y);
	uint8_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);

	int x, y = params->height;

	while(y)
	{
		for(x = 0; x != params->width; x++)
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

		alphasrc += params->source->alpha_pitch;
		src += params->source->pitch;
		dst += params->dest->pitch;
		y--;
	}
}


void (*rect_draw)(struct blit_params_t *params) = rect_draw_c;
void (*alpha_pixel_plot)(struct blit_params_t *params) = alpha_pixel_plot_c;
void (*alpha_rect_draw)(struct blit_params_t *params) = alpha_rect_draw_c;
void (*surface_blit)(struct blit_params_t *params) = surface_blit_c;
void (*alpha_surface_blit)(struct blit_params_t *params) = alpha_surface_blit_c;
void (*alpha_surface_alpha_blit)(struct blit_params_t *params) = alpha_surface_alpha_blit_c;
void (*surface_alpha_blit)(struct blit_params_t *params) = surface_alpha_blit_c;
void (*surface_alpha_surface_blit)(struct blit_params_t *params) = surface_alpha_surface_blit_c;


int clip_blit_coords(struct blit_params_t *params)
{
	if(params->width == 0 || params->height == 0)
		return 0;

	if((params->dest_x + params->width <= 0) || (params->dest_y + params->height <= 0) ||
		(params->dest_x >= params->dest->width) || (params->dest_y >= params->dest->height))
		return 0;

	if(params->dest_x < 0)
	{
		params->source_x -= params->dest_x;
		params->width += params->dest_x;
		params->dest_x = 0;
	}

	if(params->dest_x + params->width > params->dest->width)
		params->width = params->dest->width - params->dest_x;

	if(params->dest_y < 0)
	{
		params->source_y -= params->dest_y;
		params->height += params->dest_y;
		params->dest_y = 0;
	}

	if(params->dest_y + params->height > params->dest->height)
		params->height = params->dest->height - params->dest_y;

	return 1;
}


void plot_pixel(struct blit_params_t *params)
{
	if(params->dest_x < 0 || params->dest_x >= params->dest->width ||
		params->dest_y < 0 || params->dest_y >= params->dest->height)
		return;

	uint16_t *dst = get_pixel_addr(params->dest, params->dest_x, params->dest_y);
	*dst = params->colour16;
}


void draw_rect(struct blit_params_t *params)
{
	if(!clip_blit_coords(params))
		return;

	rect_draw(params);
}


void plot_alpha_pixel(struct blit_params_t *params)
{
	if(params->dest_x < 0 || params->dest_x >= params->dest->width ||
		params->dest_y < 0 || params->dest_y >= params->dest->height)
		return;

	alpha_pixel_plot(params);
}


void draw_alpha_rect(struct blit_params_t *params)
{
	if(!clip_blit_coords(params))
		return;

	alpha_rect_draw(params);
}


void blit_partial_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	if(clip_blit_coords(params))
		surface_blit(params);
}


void blit_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;
	
	params->source_x = 0;
	params->source_y = 0;
	params->width = params->source->width;
	params->height = params->source->height;

	if(clip_blit_coords(params))
		surface_blit(params);
}


void blit_partial_alpha_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	if(clip_blit_coords(params))
		alpha_surface_blit(params);
}


void blit_alpha_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	params->source_x = 0;
	params->source_y = 0;
	params->width = params->source->width;
	params->height = params->source->height;

	if(clip_blit_coords(params))
		alpha_surface_blit(params);
}


void alpha_blit_partial_alpha_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	if(clip_blit_coords(params))
		alpha_surface_alpha_blit(params);
}


void alpha_blit_alpha_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	params->source_x = 0;
	params->source_y = 0;
	params->width = params->source->width;
	params->height = params->source->height;

	if(clip_blit_coords(params))
		alpha_surface_alpha_blit(params);
}


void alpha_blit_partial_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	if(clip_blit_coords(params))
		surface_alpha_blit(params);
}


void alpha_blit_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	params->source_x = 0;
	params->source_y = 0;
	params->width = params->source->width;
	params->height = params->source->height;

	if(clip_blit_coords(params))
		surface_alpha_blit(params);
}


void alpha_surface_blit_partial_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	if(clip_blit_coords(params))
		surface_alpha_surface_blit(params);
}


void alpha_surface_blit_surface(struct blit_params_t *params)
{
	if(!params->source)
		return;

	params->source_x = 0;
	params->source_y = 0;
	params->width = params->source->width;
	params->height = params->source->height;

	if(clip_blit_coords(params))
		surface_alpha_surface_blit(params);
}
