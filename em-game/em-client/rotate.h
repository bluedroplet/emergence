struct rotate_t
{
	struct surface_t *in_surface, **out_surface;
	float red, green, blue, alpha;
	
	struct rotate_t *next;
};

void do_rotate(struct rotate_t *rotate0, int out_width, int out_height, int frames);
