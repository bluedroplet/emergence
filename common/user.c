/*
	Copyright (C) 1998-2004 Jonathan Brown
	
    This file is part of em-tools.
	
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

#define _GNU_SOURCE
#define _REENTRANT

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <pwd.h>
#include <unistd.h>

#include "../common/types.h"
#include "../common/stringbuf.h"

#ifdef EMGAME
#include "../em-client/console.h"
#endif

struct string_t *username = NULL, *home_dir = NULL, *emergence_home_dir = NULL;

void init_user()
{
	struct passwd *passwd =  getpwuid(getuid());
//	if(!passwd)
//		client_libc_error("Couldn't find user");
	
	username = new_string_text(passwd->pw_name);
	
	#ifdef EMGAME
	console_print("User: %s%c", username->text, '\n');
	#endif
	
	home_dir = new_string_text(passwd->pw_dir);
	
	#ifdef EMGAME
	console_print("Home Directory: %s%c", home_dir->text, '\n');
	#endif
	
	emergence_home_dir = new_string_text("%s/.emergence", passwd->pw_dir);
	
	struct stat s;
		
	if(stat(emergence_home_dir->text, &s))
	{
		mkdir(emergence_home_dir->text, S_IRWXU);
	}
	
	struct string_t *a = new_string_string(emergence_home_dir);
	
	string_cat_text(a, "/skins");
	
	if(stat(a->text, &s))
	{
		mkdir(a->text, S_IRWXU);
	}
	
	
	string_clear(a);
	string_cat_string(a, emergence_home_dir);
	
	string_cat_text(a, "/maps");
	
	if(stat(a->text, &s))
	{
		mkdir(a->text, S_IRWXU);
	}
	
	free_string(a);
}
