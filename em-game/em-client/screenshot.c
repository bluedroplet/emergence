#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#include "../common/types.h"
#include "../gsub/gsub.h"
#include "render.h"
#include "entry.h"

void take_screenshot()
{
	struct surface_t *shot = duplicate_surface_to_24bit(s_backbuffer);
	
	int n;
	for(n = 0; n < 1000; n++)
	{
		char filename[15];
		struct stat buf;
		sprintf(filename, "em-shot%03u.png", n);
		if(stat(filename, &buf) == -1)
		{
			write_png_surface(shot, filename);
			break;
		}
	}
	
	free_surface(shot);
}
