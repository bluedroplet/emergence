#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "../common/types.h"
#include "../shared/cvar.h"
#include "../gsub/gsub.h"
#include "game.h"
#include "render.h"

double star_zs = 1.0f;

struct star_t
{
	float x, y, z;
	uint8_t alpha;

} *stars = NULL;

int numstars = 0;


/*
int qc_star_density(float density)
{
	numstars = (int)floor(((float)(mapwidth * mapheight * 32 * 32) * density));

	m_free(stars);

	if(!numstars)
	{
		return 1;
	}

	stars = (star_t*)m_alloc(numstars * sizeof(star_t));

	for(int i = 0; i != numstars; i++)
	{
		stars[i].z = rand_float() * star_zs * 2;
		stars[i].x = rand_float() * mapwidth * 32;
		stars[i].y = rand_float() * mapheight * 32;

		stars[i].colour = vid_graylookup[lround((rand_float() * 255.0f))];
	}

	return 1;
}


void init_stars()
{
	qc_StarDensity(get_cvar_float("r_StarDensity"));

	SetCvarQCFunction("r_StarDensity", qc_StarDensity);
}
*/

void init_stars()
{
	numstars = 1000;
	
	stars = malloc(numstars * sizeof(struct star_t));

	int s;
	for(s = 0; s < numstars; s++)
	{
		stars[s].z = drand48() * star_zs + 1.0f;
		stars[s].x = drand48() * 8000;
		stars[s].y = drand48() * 8000;

		stars[s].alpha = lround(drand48() * 255.0f);
	}
}


void render_stars()
{
	if(!numstars)
		return;

	int s;
	
	struct blit_params_t params;
		
	params.dest = s_backbuffer;

	for(s = 0; s < numstars; s++)
	{
		int x = (int)floor((stars[s].x - viewx) * star_zs / stars[s].z * ((double)(vid_width) / 1600.0)) + vid_width / 2;
		int y = vid_height / 2 - 1 - (int)floor((stars[s].y - viewy) * star_zs / stars[s].z * ((double)(vid_width) / 1600.0));

		if(y < 0 || y >= vid_height)
		{
			continue;
		}

		if(x < 0 || x >= vid_width)
		{
			continue;
		}

		
		params.red = stars[s].alpha;
		params.green = stars[s].alpha;
		params.blue = stars[s].alpha;
		
		params.dest_x = x;
		params.dest_y = y;
		
		plot_pixel(&params);
		
		params.alpha >>= 1;
		
		params.dest_x++;
		plot_pixel(&params);
		
		params.dest_x--;
		params.dest_y++;
		plot_pixel(&params);
		
		params.dest_y -= 2;
		plot_pixel(&params);
		
		params.dest_x--;
		params.dest_y++;
		plot_pixel(&params);
	}
}
