#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdint.h>


#include "../../common/types.h"
#include "../../common/stringbuf.h"
#include "parse.h"
#include "cvar.h"
#include "user.h"
#include "../console.h"

#ifdef EMCLIENT
#include "../control.h"
#endif

#ifdef LINUX
#include "../entry.h"
#endif

#ifdef WIN32
#include "../nfcl/win32/entry.h"
#endif

void exec_config_file(char *filename)
{
	if(!filename)
		return;

	if(!*filename)
		return;

	FILE *file = fopen(filename, "r");

	if(!file)
	{
		console_print("Could not execute file \"");
		console_print(filename);
		console_print("\"\n");
		return;
	}

	console_print("Executing \"");
	console_print(filename);
	console_print("\"\n");
	

	struct string_t *s = new_string();

	char c;

	do
	{
		c = fgetc(file);

		while(c != EOF && c != '\n')
		{
			string_cat_char(s, c);
			c = fgetc(file);
		}

		parse_command(s->text);

		string_clear(s);

	} while(c != EOF);

	fclose(file);

	free_string(s);
}


void write_config_file()
{
	struct string_t *string = new_string_string(emergence_home_dir);
	string_cat_text(string, "/nfcl.cfg");
	
	FILE *file = fopen(string->text, "w");
	
	free_string(string);

	if(!file)
		return;

	fputs("// do not put commands other than bind in this file\n\n", file);

	dump_cvars(file);
	
	#ifdef EMCLIENT
	dump_bindings(file);
	#endif

	fclose(file);
}
