struct particle_t
{
	float	xpos, ypos;
	float	xvel, yvel;
	float	creation, last;
};

void init_particles();
void kill_particles();
void create_upper_particle(struct particle_t *particle);
void create_lower_particle(struct particle_t *particle);
void render_upper_particles();
void render_lower_particles();
