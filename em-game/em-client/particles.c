#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdint.h>

#include <math.h>


#include "../common/types.h"
#include "../common/stringbuf.h"
#include "../common/buffer.h"
#include "../common/minmax.h"
#include "../shared/rdtsc.h"
#include "../gsub/gsub.h"
#include "shared/timer.h"
#include "main.h"
#include "game.h"
#include "render.h"
#include "particles.h"



#define NUMPARTICLES 8000
#define MAX_PARTICLE_TIME 0.025


struct particle_t *upper_particles;
uint32_t last_upper_particle_time;
uint8_t upper_pinuse[NUMPARTICLES];
int nupperpart;

struct particle_t *lower_particles;
uint32_t last_lower_particle_time;
uint8_t lower_pinuse[NUMPARTICLES];
int nlowerpart;



void init_particles()
{
	upper_particles = (struct particle_t*)malloc(sizeof(struct particle_t) * NUMPARTICLES);
	lower_particles = (struct particle_t*)malloc(sizeof(struct particle_t) * NUMPARTICLES);
}


void kill_particles()
{
	free(upper_particles);
	free(lower_particles);
}


void create_upper_particle(struct particle_t *p)
{
	upper_particles[nupperpart] = *p;

	upper_pinuse[nupperpart] = 1;

	++nupperpart;
	nupperpart %= NUMPARTICLES;
}


void create_lower_particle(struct particle_t *p)
{
	lower_particles[nlowerpart] = *p;

	lower_pinuse[nlowerpart] = 1;

	++nlowerpart;
	nlowerpart %= NUMPARTICLES;
}


void render_upper_particles()
{
	int p;
	
	struct blit_params_t params;
		
	params.dest = s_backbuffer;
	params.red = 0xff;
	
	for(p = 0; p != NUMPARTICLES; p++)
	{
		int i = (p + nupperpart) % NUMPARTICLES;

		if(upper_pinuse[i])
		{
			double life = cgame_time - upper_particles[i].creation;
			
			if(life > 1.0f)
			{
				upper_pinuse[i] = 0;
				continue;
			}
			
			double delta_time = cgame_time - upper_particles[i].last;
			
			double particle_time = delta_time;
			int particle_ticks = 1;
			
		//	while(particle_time > MAX_PARTICLE_TIME)
		//		particle_time /= 2, particle_ticks++;
			

		//	while(particle_ticks--)
			{
				upper_particles[i].xvel += (drand48() - 0.5) * 2400 * particle_time;
				upper_particles[i].yvel += (drand48() - 0.5) * 2400 * particle_time;
				
				double dampening = exp(-8.0f * particle_time);
				
				upper_particles[i].xvel *= dampening;
				upper_particles[i].yvel *= dampening;
	
				upper_particles[i].xpos += upper_particles[i].xvel * particle_time;
				upper_particles[i].ypos += upper_particles[i].yvel * particle_time;
			}

			int x, y;
			
			world_to_screen(upper_particles[i].xpos, upper_particles[i].ypos, &x, &y);
			
			
			
			if(life < 0.1275)
				params.green = params.blue = (uint8_t)min(floor(life * 2000.0), 255);
			else
				params.green = params.blue = 0xff;
			
			params.alpha  = (uint8_t)(255 - floor(life * 255.0f));
			
			params.dest_x = x;
			params.dest_y = y;
			
			alpha_plot_pixel(&params);
			
			params.alpha >>= 1;
			
			params.dest_x++;
			alpha_plot_pixel(&params);
			
			params.dest_x--;
			params.dest_y++;
			alpha_plot_pixel(&params);
			
			params.dest_y -= 2;
			alpha_plot_pixel(&params);
			
			params.dest_x--;
			params.dest_y++;
			alpha_plot_pixel(&params);
			
			upper_particles[i].last = cgame_time;
		}
	}
}


void render_lower_particles()
{
	int p;
	
	struct blit_params_t params;
		
	params.dest = s_backbuffer;
	params.red = 0xff;

	for(p = 0; p != NUMPARTICLES; p++)
	{
		int i = (p + nlowerpart) % NUMPARTICLES;

		if(lower_pinuse[i])
		{
			double life = cgame_time - lower_particles[i].creation;
			
			if(life > 1.0f)
			{
				lower_pinuse[i] = 0;
				continue;
			}
			
			double delta_time = cgame_time - lower_particles[i].last;

			double particle_time = delta_time;
			int particle_ticks = 1;
			
		//	while(particle_time > MAX_PARTICLE_TIME)
		//		particle_time /= 2, particle_ticks++;
			

		//	while(particle_ticks--)
			{
				lower_particles[i].xvel += (drand48() - 0.5) * 2400 * particle_time;
				lower_particles[i].yvel += (drand48() - 0.5) * 2400 * particle_time;
				
				double dampening = exp(-8.0f * particle_time);
				
				lower_particles[i].xvel *= dampening;
				lower_particles[i].yvel *= dampening;
				
				lower_particles[i].xpos += lower_particles[i].xvel * particle_time;
				lower_particles[i].ypos += lower_particles[i].yvel * particle_time;
			}

			int x, y;
			
			world_to_screen(lower_particles[i].xpos, lower_particles[i].ypos, &x, &y);
			
			if(life < 0.1275)
				params.green = params.blue = (uint8_t)min(floor(life * 2000.0), 255);
			else
				params.green = params.blue = 0xff;
			
			params.alpha  = (uint8_t)(255 - floor(life * 255.0f));
			
			params.dest_x = x;
			params.dest_y = y;
			
			alpha_plot_pixel(&params);
			
			params.alpha >>= 1;
			
			params.dest_x++;
			alpha_plot_pixel(&params);
			
			params.dest_x--;
			params.dest_y++;
			alpha_plot_pixel(&params);
			
			params.dest_y -= 2;
			alpha_plot_pixel(&params);
			
			params.dest_x--;
			params.dest_y++;
			alpha_plot_pixel(&params);
			
			lower_particles[i].last = cgame_time;
		}
	}
}
