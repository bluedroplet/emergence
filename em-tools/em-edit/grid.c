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
#include <math.h>

#include "../gsub/gsub.h"
#include "nodes.h"
#include "interface.h"

int grid_granularity = 0;
double grid_spacing = 20.0;

void draw_grid()
{
	int startx, starty, endx, endy;

	double this_grid_spacing = exp2(floor(log(1 / zoom) / log(2))) * grid_spacing * exp2(grid_granularity);

	double world_x, world_y;

	screen_to_world(0, vid_height - 1, &world_x, &world_y);

	startx = (int)(world_x / this_grid_spacing);
	starty = (int)(world_y / this_grid_spacing);

	screen_to_world(vid_width - 1, 0, &world_x, &world_y);

	endx = (int)(world_x / this_grid_spacing) + 1;
	endy = (int)(world_y / this_grid_spacing) + 1;

	int x, y;
	blit_colour = 0xffff;

	for(x = startx; x != endx; x++)
		for(y = starty; y != endy; y++)
		{
			world_to_screen(x * this_grid_spacing, y * this_grid_spacing, &blit_destx, &blit_desty);
			plot_pixel();
		}
}


void snap_to_grid(double inx, double iny, double *outx, double *outy)
{
	double this_grid_spacing = exp2(floor(log(1 / zoom) / log(2))) * grid_spacing * exp2(grid_granularity);
	
	if(view_state & VIEW_GRID)
	{
		*outx = (round(inx / this_grid_spacing)) * this_grid_spacing;
		*outy = (round(iny / this_grid_spacing)) * this_grid_spacing;
	}
	else
	{
		*outx = inx;
		*outy = iny;
	}
}

void decrease_grid_granularity()
{
	if(grid_granularity > -3)
		grid_granularity--;
}

void increase_grid_granularity()
{
	if(grid_granularity < 40000)
		grid_granularity++;
}
