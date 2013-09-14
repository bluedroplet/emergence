/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifndef _INC_PARTICLES
#define _INC_PARTICLES

struct particle_t
{
	float xpos, ypos;
	float xvel, yvel;
	float creation, last;
	uint8_t start_red, start_green, start_blue;
	uint8_t end_red, end_green, end_blue;
};

void init_particles();
void kill_particles();
void clear_particles();
void create_upper_particle(struct particle_t *particle);
void create_lower_particle(struct particle_t *particle);
void render_upper_particles();
void render_lower_particles();

#endif
