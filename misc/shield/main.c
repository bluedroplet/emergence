/* 
	Copyright (C) 1998-2002 Jonathan Brown

    This file is part of em-rotate.

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
#include <assert.h>
#include <math.h>


#include "../common/minmax.h"
#include "../gsub/gsub.h"



#define WIDTH 58
#define MAX_DIST ((double)WIDTH / 2.0)

#define SUB 100

int main (int argc, char **argv)
{
	struct surface_t *shield = new_surface(SURFACE_ALPHA8BIT, WIDTH, WIDTH);
	clear_surface(shield);
		
	int x, y, sx, sy;
	
	for(y = 0; y < WIDTH; y++)
	{
		for(x = 0; x < WIDTH; x++)
		{
			double i = 0.0;
			
			for(sy = 0; sy < SUB; sy++)
			{
				for(sx = 0; sx < SUB; sx++)
				{
					double xdist = (double)x - (double)WIDTH / 2.0 + ((double)sx + 0.5) / (double)SUB;
					double ydist = (double)y - (double)WIDTH / 2.0 + ((double)sy + 0.5) / (double)SUB;
					
					double dist = hypot(xdist, ydist);
					
					if(dist > MAX_DIST)
						continue;
					
					double t = cos((dist / MAX_DIST) * M_PI_2);
					i += 1.0 - min(max(t, 0.0), 1.0);
				}
				
			}
			
			((uint8_t*)shield->alpha_buf)[y * WIDTH + x] = lround((i / (SUB*SUB)) * 255.0);
		}
	}
	
	write_png_surface(shield, "shield.png");
		
	exit(0);
}


