#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../common/llist.h"
#include "../common/stringbuf.h"
#include "../gsub/gsub.h"


struct ris_t
{
	struct string_t *filename;
	struct surface_t *surface;
		
	struct ris_t *next;
		
} *ris0 = NULL;

float ris_multiplier = 0.64;
	

void set_ri_surface_multiplier(float m)
{
	ris_multiplier = m;
	
	struct ris_t *cris = ris0;
		
	while(cris)
	{
		free_surface(cris->surface);
		
		struct surface_t *temp = read_png_surface(cris->filename->text);
		
		cris->surface = resize(temp, temp->width * ris_multiplier, 
			temp->height * ris_multiplier, NULL);
		
		free_surface(temp);		
		
		cris = cris->next;
	}
}


struct surface_t *load_ri_surface(char *filename)
{
	struct ris_t ris;
		
	ris.filename = new_string_text(filename);
	
	struct surface_t *temp = read_png_surface(filename);
	
	
	ris.surface = resize(temp, temp->width * ris_multiplier, 
		temp->height * ris_multiplier, NULL);
	
	free_surface(temp);
	
	LL_ADD(struct ris_t, &ris0, &ris);
		
	return ris.surface;
}


void free_ri_surface(struct surface_t *ris)
{
	free_surface(ris);
	
	struct ris_t *cris = ris0;
		
	while(cris)
	{
		if(cris->surface == ris)
		{
			LL_REMOVE(struct ris_t, &ris0, cris);
			break;
		}

		cris = cris->next;
	}
}


void kill_ris()
{
	while(ris0)
	{
		free_surface(ris0->surface);
		
		LL_REMOVE(struct ris_t, &ris0, ris0);
	}
}
