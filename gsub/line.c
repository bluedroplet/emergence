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

#include "gsub.h"


void draw_horiz_run(struct blit_params_t *params, uint16_t **dest, int xadvance, int runlength)
{
	uint16_t *cdest = *dest;

	while(runlength)
	{
		*cdest = params->colour16;
		cdest += xadvance;
		runlength--;
	}

	*dest = (uint16_t*)((uint8_t*)cdest + params->dest->pitch);
}


void draw_vert_run(struct blit_params_t *params, uint16_t **dest, int xadvance, int runlength)
{
	uint16_t *cdest = *dest;

	while(runlength)
	{
		*cdest = params->colour16;
		(uint8_t*)cdest += params->dest->pitch;
		runlength--;
	}
	
	*dest = cdest + xadvance;
}


void draw_line(struct blit_params_t *params)
{
	int temp, i, adjup, adjdown, errorterm, xadvance, xdelta, ydelta, 
		wholestep, initialpixelcount, finalpixelcount, runlength;

	if(params->y1 > params->y2)
	{
		temp = params->y1;
		params->y1 = params->y2;
		params->y2 = temp;

		temp = params->x1;
		params->x1 = params->x2;
		params->x2 = temp;
	}

	if((params->y2 < 0) || (params->y1 >= params->dest->height))
		return;

	if(params->y1 < 0)
	{
		params->x1 -= ((params->x2 - params->x1) * params->y1) / (params->y2 - params->y1);
		params->y1 = 0;
	}

	if(params->y2 >= params->dest->height)
	{
		params->x2 -= ((params->x2 - params->x1) * (params->y2 - params->dest->height + 1)) / (params->y2 - params->y1);
		params->y2 = params->dest->height - 1;
	}

	if(params->x1 < params->x2)
	{
		if((params->x2 < 0) || (params->x1 >= params->dest->width))
			return;

		if(params->x1 < 0)
		{
			params->y1 -= ((params->y2 - params->y1) * params->x1) / (params->x2 - params->x1);
			params->x1 = 0;
		}

		if(params->x2 >= params->dest->width)
		{
			params->y2 -= ((params->y2 - params->y1) * (params->x2 - params->dest->width + 1)) / (params->x2 - params->x1);
			params->x2 = params->dest->width - 1;
		}
	}
	else
	{
		if((params->x1 < 0) || (params->x2 >= params->dest->width))
			return;

		if(params->x2 < 0)
		{
			params->y2 += ((params->y2 - params->y1) * params->x2) / (params->x1 - params->x2);
			params->x2 = 0;
		}

		if(params->x1 >= params->dest->width)
		{
			params->y1 += ((params->y2 - params->y1) * (params->x1 - params->dest->width + 1)) / (params->x1 - params->x2);
			params->x1 = params->dest->width - 1;
		}
	}

	uint16_t *dest = get_pixel_addr(params->dest, params->x1, params->y1);

	if((xdelta = params->x2 - params->x1) < 0)
	{
		xadvance = -1;
		xdelta = -xdelta;
	}
	else
	{
		xadvance = 1;
	}

	ydelta = params->y2 - params->y1;

	if(xdelta == 0)
	{
		for(i = 0; i <= ydelta; i++)
		{
			*dest = params->colour16;
			(uint8_t*)dest += params->dest->pitch;
		}

		return;
	}

	if(ydelta == 0)
	{
		for(i = 0; i <= xdelta; i++)
		{
			*dest = params->colour16;
			dest += xadvance;
		}

		return;
	}

	if(xdelta == ydelta)
	{
		for(i = 0; i <= xdelta; i++)
		{
			*dest = params->colour16;
			(uint8_t*)dest += xadvance * 2 + params->dest->pitch;
		}

		return;
	}

	if(xdelta >= ydelta)
	{
		wholestep = xdelta / ydelta;
		adjup = (xdelta % ydelta) * 2;
		adjdown = ydelta * 2;
		errorterm = (xdelta % ydelta) - (ydelta * 2);
		initialpixelcount = (wholestep / 2) + 1;
		finalpixelcount = initialpixelcount;

		if((adjup == 0) && ((wholestep & 1) == 0))
			initialpixelcount--;

		if((wholestep & 0x01) != 0)
			errorterm += ydelta;

		draw_horiz_run(params, &dest, xadvance, initialpixelcount);

		for(i = 0; i < (ydelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_horiz_run(params, &dest, xadvance, runlength);
		}

		draw_horiz_run(params, &dest, xadvance, finalpixelcount);

		return;
	}
	else
	{
		wholestep = ydelta / xdelta;
		adjup = (ydelta % xdelta) * 2;
		adjdown = xdelta * 2;
		errorterm = (ydelta % xdelta) - (xdelta * 2);
		initialpixelcount = (wholestep / 2) + 1;
		finalpixelcount = initialpixelcount;

		if((adjup == 0) && ((wholestep & 1) == 0))
			initialpixelcount--;

		if((wholestep & 0x01) != 0)
			errorterm += xdelta;

		draw_vert_run(params, &dest, xadvance, initialpixelcount);

		for(i = 0; i < (xdelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_vert_run(params, &dest, xadvance, runlength);
		}

		draw_vert_run(params, &dest, xadvance, finalpixelcount);			

		return;
	}
}

