#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/stat.h>

#include "stringbuf.h"
#include "prefix.h"

//__thread char *found_resource = NULL;		// doesn't work with old glibc
char *found_resource = NULL;				// this is not thread safe

char *find_resource(char *resource)
{
	struct stat buf;
	struct string_t *s;
		
	char *bindir = br_extract_dir(SELFPATH);
	
	s = new_string_text(bindir);
	string_cat_text(s, "/../share/emergence/");	// LSB install
	string_cat_text(s, resource);
	
	if(stat(s->text, &buf) == 0)
		goto found;
	
	
	free_string(s);
	s = new_string_text(bindir);
	string_cat_text(s, "/");				// home dir install
	string_cat_text(s, resource);
	
	if(stat(s->text, &buf) == 0)
		goto found;
	
	
	free_string(s);
	s = new_string_text(bindir);
	string_cat_text(s, "/../share/");					// run from build dir
	string_cat_text(s, resource);
	
	if(stat(s->text, &buf) < 0)
		goto error;
	
found:
	
	free(bindir);
	free(found_resource);
	printf("Found resource: %s\n", s->text);
	found_resource = s->text;
	free(s);
	
	return found_resource;
	
error:
	
	printf("Could not find resource: %s\n", resource);
	free(bindir);
	free_string(s);
	return NULL;
}
