/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
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



#define WIDTH 25
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
					i += min(max(t, 0.0), 1.0);
				}
				
			}
			
			((uint8_t*)shield->alpha_buf)[y * WIDTH + x] = lround((i / (SUB*SUB)) * 255.0);
		}
	}
	
	write_png_surface(shield, "plasma.png");
		
	exit(0);
}
