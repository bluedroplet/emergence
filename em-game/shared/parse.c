/*
	Copyright (C) 1998-2002 Jonathan Brown
	
    This file is part of emergence.
	
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source distribution.
	
	Jonathan Brown
	jbrown@emergence.uk.net
*/


#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../../common/types.h"
#include "../../common/stringbuf.h"
#include "cvar.h"

void console_print(char *text);


void parse_command(char *cmd)
{
	char *token = strtok(cmd, " ");

	if(!token)
		return;

	if(strcmp(token, "//") == 0)
		return;
	
	struct string_t *output = new_string();
	
	if(is_cvar(token))
	{
		char *val;

		switch(get_cvar_type(token))
		{
		case CVAR_INT:
			val = strtok(NULL, " ");

			if(val)
				set_cvar_int(token, atoi(val));
			else
			{
				string_cat_text(output, token);
				string_cat_text(output, " is ");
				string_cat_int(output, get_cvar_int(token));
				string_cat_char(output, '\n');

				console_print(output->text);
			}

			break;


		case CVAR_FLOAT:
			val = strtok(NULL, " ");

			if(val)
				set_cvar_float(token, (float)atof(val));
			else
			{
				string_cat_text(output, token);
				string_cat_text(output, " is ");
				string_cat_double(output, get_cvar_float(token), 4);
				string_cat_char(output, '\n');

				console_print(output->text);
			}

			break;


		case CVAR_STRING:
			val = strtok(NULL, " ");

			if(val)
				set_cvar_string(token, val);
			else
			{
				string_cat_text(output, token);
				string_cat_text(output, " is ");
				string_cat_text(output, get_cvar_string(token));
				string_cat_char(output, '\n');

				console_print(output->text);
			}

			break;


		case CVAR_CMD:
			val = strtok(NULL, "");
			execute_cvar_command(token, val);

			break;
		}

		free_string(output);

		return;
	}


	string_cat_text(output, "unknown command: \"");
	string_cat_text(output, token);
	string_cat_text(output, "\"\n");

	console_print(output->text);

	free_string(output);
}
