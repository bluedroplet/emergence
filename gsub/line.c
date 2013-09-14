/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifdef LINUX
#define _GNU_SOURCE
#ifndef _REENTRANT
#define _REENTRANT
#endif
#endif

#include <stdint.h>

#include "gsub.h"


void draw_horiz_run_888P8(struct blit_params_t *params, uint8_t **dest, int xadvance, int runlength)
{
	uint8_t *cdest = *dest;

	while(runlength)
	{
		cdest[0] = params->blue;
		cdest[1] = params->green;
		cdest[2] = params->red;
		cdest += xadvance;
		runlength--;
	}

	*dest = cdest + params->dest->pitch;
}


void draw_vert_run_888P8(struct blit_params_t *params, uint8_t **dest, int xadvance, int runlength)
{
	uint8_t *cdest = *dest;

	while(runlength)
	{
		cdest[0] = params->blue;
		cdest[1] = params->green;
		cdest[2] = params->red;
		cdest += params->dest->pitch;
		runlength--;
	}
	
	*dest = cdest + xadvance;
}


void draw_line_888P8(struct blit_params_t *params)
{
	int i, adjup, adjdown, errorterm, xadvance, xdelta, ydelta, 
		wholestep, initialpixelcount, finalpixelcount, runlength;

	uint8_t *dest = get_pixel_addr(params->dest, params->x1, params->y1);

	if((xdelta = params->x2 - params->x1) < 0)
	{
		xadvance = -4;
		xdelta = -xdelta;
	}
	else
	{
		xadvance = 4;
	}

	ydelta = params->y2 - params->y1;

	if(xdelta == 0)
	{
		for(i = 0; i <= ydelta; i++)
		{
			dest[0] = params->red;
			dest[1] = params->green;
			dest[2] = params->blue;
			dest += params->dest->pitch;
		}

		return;
	}

	if(ydelta == 0)
	{
		for(i = 0; i <= xdelta; i++)
		{
			dest[0] = params->red;
			dest[1] = params->green;
			dest[2] = params->blue;
			dest += xadvance;
		}

		return;
	}

	if(xdelta == ydelta)
	{
		for(i = 0; i <= xdelta; i++)
		{
			dest[0] = params->red;
			dest[1] = params->green;
			dest[2] = params->blue;
			dest += xadvance + params->dest->pitch;
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

		draw_horiz_run_888P8(params, &dest, xadvance, initialpixelcount);

		for(i = 0; i < (ydelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_horiz_run_888P8(params, &dest, xadvance, runlength);
		}

		draw_horiz_run_888P8(params, &dest, xadvance, finalpixelcount);

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

		draw_vert_run_888P8(params, &dest, xadvance, initialpixelcount);

		for(i = 0; i < (xdelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_vert_run_888P8(params, &dest, xadvance, runlength);
		}

		draw_vert_run_888P8(params, &dest, xadvance, finalpixelcount);			

		return;
	}
}


void draw_horiz_run_888(struct blit_params_t *params, uint8_t **dest, int xadvance, int runlength)
{
	uint8_t *cdest = *dest;

	while(runlength)
	{
		cdest[0] = params->red;
		cdest[1] = params->green;
		cdest[2] = params->blue;
		cdest += xadvance;
		runlength--;
	}

	*dest = cdest + params->dest->pitch;
}


void draw_vert_run_888(struct blit_params_t *params, uint8_t **dest, int xadvance, int runlength)
{
	uint8_t *cdest = *dest;

	while(runlength)
	{
		cdest[0] = params->red;
		cdest[1] = params->green;
		cdest[2] = params->blue;
		cdest += params->dest->pitch;
		runlength--;
	}
	
	*dest = cdest + xadvance;
}


void draw_line_888(struct blit_params_t *params)
{
	int i, adjup, adjdown, errorterm, xadvance, xdelta, ydelta, 
		wholestep, initialpixelcount, finalpixelcount, runlength;

	uint8_t *dest = get_pixel_addr(params->dest, params->x1, params->y1);

	if((xdelta = params->x2 - params->x1) < 0)
	{
		xadvance = -3;
		xdelta = -xdelta;
	}
	else
	{
		xadvance = 3;
	}

	ydelta = params->y2 - params->y1;

	if(xdelta == 0)
	{
		for(i = 0; i <= ydelta; i++)
		{
			dest[0] = params->red;
			dest[1] = params->green;
			dest[2] = params->blue;
			dest += params->dest->pitch;
		}

		return;
	}

	if(ydelta == 0)
	{
		for(i = 0; i <= xdelta; i++)
		{
			dest[0] = params->red;
			dest[1] = params->green;
			dest[2] = params->blue;
			dest += xadvance;
		}

		return;
	}

	if(xdelta == ydelta)
	{
		for(i = 0; i <= xdelta; i++)
		{
			dest[0] = params->red;
			dest[1] = params->green;
			dest[2] = params->blue;
			dest += xadvance + params->dest->pitch;
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

		draw_horiz_run_888(params, &dest, xadvance, initialpixelcount);

		for(i = 0; i < (ydelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_horiz_run_888(params, &dest, xadvance, runlength);
		}

		draw_horiz_run_888(params, &dest, xadvance, finalpixelcount);

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

		draw_vert_run_888(params, &dest, xadvance, initialpixelcount);

		for(i = 0; i < (xdelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_vert_run_888(params, &dest, xadvance, runlength);
		}

		draw_vert_run_888(params, &dest, xadvance, finalpixelcount);			

		return;
	}
}


void draw_horiz_run_565(struct blit_params_t *params, uint16_t **dest, int xadvance, 
	int runlength, uint16_t colour)
{
	uint16_t *cdest = *dest;

	while(runlength)
	{
		*cdest = colour;
		cdest += xadvance;
		runlength--;
	}

	*dest = (uint16_t*)((uint8_t*)cdest + params->dest->pitch);
}


void draw_vert_run_565(struct blit_params_t *params, uint16_t **dest, int xadvance, 
	int runlength, uint16_t colour)
{
	uint16_t *cdest = *dest;

	while(runlength)
	{
		*cdest = colour;
		cdest = (uint16_t*)&((uint8_t*)cdest)[params->dest->pitch];
		runlength--;
	}
	
	*dest = cdest + xadvance;
}


void draw_line_565(struct blit_params_t *params)
{
	int i, adjup, adjdown, errorterm, xadvance, xdelta, ydelta, 
		wholestep, initialpixelcount, finalpixelcount, runlength;

	uint16_t colour = convert_24bit_to_16bit(params->red, params->green, params->blue);
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
			*dest = colour;
			dest = (uint16_t*)&((uint8_t*)dest)[params->dest->pitch];
		}

		return;
	}

	if(ydelta == 0)
	{
		for(i = 0; i <= xdelta; i++)
		{
			*dest = colour;
			dest += xadvance;
		}

		return;
	}

	if(xdelta == ydelta)
	{
		for(i = 0; i <= xdelta; i++)
		{
			*dest = colour;
			dest = (uint16_t*)&((uint8_t*)dest)[xadvance * 2 + params->dest->pitch];
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

		draw_horiz_run_565(params, &dest, xadvance, initialpixelcount, colour);

		for(i = 0; i < (ydelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_horiz_run_565(params, &dest, xadvance, runlength, colour);
		}

		draw_horiz_run_565(params, &dest, xadvance, finalpixelcount, colour);

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

		draw_vert_run_565(params, &dest, xadvance, initialpixelcount, colour);

		for(i = 0; i < (xdelta - 1); i++)
		{
			runlength = wholestep;

			if((errorterm += adjup) > 0)
			{
				runlength++;
				errorterm -= adjdown;
			}

			draw_vert_run_565(params, &dest, xadvance, runlength, colour);
		}

		draw_vert_run_565(params, &dest, xadvance, finalpixelcount, colour);			

		return;
	}
}


void draw_line(struct blit_params_t *params)
{
	int temp;

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
		params->x2 -= ((params->x2 - params->x1) * (params->y2 - params->dest->height + 1)) / 
			(params->y2 - params->y1);
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
			params->y2 -= ((params->y2 - params->y1) * (params->x2 - params->dest->width + 1)) / 
				(params->x2 - params->x1);
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
			params->y1 += ((params->y2 - params->y1) * (params->x1 - params->dest->width + 1)) / 
				(params->x1 - params->x2);
			params->x1 = params->dest->width - 1;
		}
	}

	switch(params->dest->flags)
	{
	case SURFACE_24BITPADDING8BIT:
		return draw_line_888P8(params);
		
	case SURFACE_24BIT:
	case SURFACE_24BITALPHA8BIT:
		return draw_line_888(params);

	case SURFACE_16BIT:
	case SURFACE_16BITALPHA8BIT:
		return draw_line_565(params);
	}
}
