struct ris_t
{
	struct string_t *filename;
	struct surface_t *surface;
		
	struct ris_t *next;
		
};

void set_ri_surface_multiplier(float m);
struct ris_t *load_ri_surface(char *filename);
void free_ri_surface(struct ris_t *ris);
void kill_ris();
