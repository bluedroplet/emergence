#define _GNU_SOURCE
#define _REENTRANT

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <pwd.h>
#include <unistd.h>

#include "../../common/types.h"
#include "../../common/stringbuf.h"
#include "../console.h"

struct string_t *username = NULL, *home_dir = NULL, *emergence_home_dir = NULL;

void init_user()
{
	struct passwd *passwd =  getpwuid(getuid());
//	if(!passwd)
//		client_libc_error("Couldn't find user");
	
	username = new_string_text(passwd->pw_name);
	console_print("User: %s%c", username->text, '\n');
	home_dir = new_string_text(passwd->pw_dir);
	console_print("Home Directory: %s%c", home_dir->text, '\n');
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
