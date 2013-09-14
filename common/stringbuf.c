/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
 */

#ifdef LINUX
#define _GNU_SOURCE
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include <zlib.h>

#include "stringbuf.h"


struct string_t *new_string()
{
	struct string_t *string = malloc(sizeof(struct string_t));
	string->text = malloc(1);
	string->text[0] = '\0';

	return string;
}


struct string_t *new_string_string(struct string_t *cat_string)
{
	struct string_t *string = malloc(sizeof(struct string_t));
	string->text = malloc(1);
	string->text[0] = '\0';

	string_cat_string(string, cat_string);

	return string;
}


struct string_t *new_string_text(const char *fmt, ...)
{
	struct string_t *string = malloc(sizeof(struct string_t));
	string->text = malloc(1);
	string->text[0] = '\0';

	char *text;
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&text, fmt, ap);
	va_end(ap);
	
	string_cat_text(string, text);
	
	free(text);

	return string;
}


struct string_t *new_string_int(int val)
{
	struct string_t *string = malloc(sizeof(struct string_t));
	string->text = malloc(1);
	string->text[0] = '\0';

	string_cat_int(string, val);

	return string;
}


struct string_t *new_string_uint32(uint32_t val)
{
	struct string_t *string = malloc(sizeof(struct string_t));
	string->text = malloc(1);
	string->text[0] = '\0';

	string_cat_uint32(string, val);

	return string;
}


struct string_t *new_string_double(double val, int digits)
{
	struct string_t *string = malloc(sizeof(struct string_t));
	string->text = malloc(1);
	string->text[0] = '\0';

	string_cat_double(string, val, digits);

	return string;
}


struct string_t *new_string_uint64(uint64_t val)
{
	struct string_t *string = malloc(sizeof(struct string_t));
	string->text = malloc(1);
	string->text[0] = '\0';

	string_cat_uint64(string, val);

	return string;
}


void free_string(struct string_t *string)
{
	if(!string)
		return;

	free(string->text);
	free(string);
}


void string_cat_string(struct string_t *string, struct string_t *cat_string)
{
	if(!string)
		return;

	if(!cat_string)
		return;

	string_cat_text(string, cat_string->text);
}


void string_cat_text(struct string_t *string, const char *fmt, ...)
{
	if(!string)
		return;
	
	if(!fmt)
		return;
	
	char *cat_text;
	va_list ap;
	
	va_start(ap, fmt);
	vasprintf(&cat_text, fmt, ap);
	va_end(ap);
	
	char *temp = malloc(strlen(string->text) + strlen(cat_text) + 1);
	
	strcpy(temp, string->text);
	strcat(temp, cat_text);
	
	free(string->text);
	
	string->text = temp;
	
	free(cat_text);
}


void string_cat_char(struct string_t *string, char c)
{
	if(!string)
		return;

	int len = strlen(string->text);

	char *temp = malloc(len + 2);

	strcpy(temp, string->text);

	temp[len++] = c;
	temp[len] = '\0';

	free(string->text);

	string->text = temp;
}


void string_cat_int(struct string_t *string, int val)
{
	if(!string)
		return;

#ifdef LINUX

	char *newtext;

	asprintf(&newtext, "%i", val);

	string_cat_text(string, newtext);

	free(newtext);

#endif

#ifdef WIN32

	char newtext[20];

	string_cat(string, itoa(val, newtext, 10));

#endif
}

void string_cat_uint32(struct string_t *string, uint32_t val)
{
#ifdef LINUX

	char *newtext;

	asprintf(&newtext, "%u", (unsigned int)val);

	string_cat_text(string, newtext);

	free(newtext);

#endif

#ifdef WIN32

	char newtext[20];

	string_cat(string, ultoa(val, newtext, 10));

#endif
}


void string_cat_double(struct string_t *string, double val, int digits)
{
#ifdef LINUX

	char *newtext;

	asprintf(&newtext, "%.2f", val);

	string_cat_text(string, newtext);

	free(newtext);

#endif

#ifdef WIN32

	char newtext[50];

	cat(_gcvt(val, digits, newtext));

#endif
}

void string_cat_uint64(struct string_t *string, uint64_t val)
{
#ifdef LINUX

//	char *newtext;

//	asprintf(&newtext, "%ull", val);

//	cat(newtext);

//	free(newtext);

#endif

#ifdef WIN32

	char newtext[20];

	cat(_ui64toa(val, newtext, 10));

#endif
}


void fwrite_string(struct string_t *string, FILE *file)
{
	if(!string)
		return;

	char *cc = string->text;
	
	while(*cc)
		fwrite(cc++, 1, 1, file);

	fwrite(cc, 1, 1, file);
}


struct string_t *fread_string(FILE *file)
{
	struct string_t *string = new_string();
	
	char cc;
	
	while(fread(&cc, 1, 1, file) == 1)
	{
		if(!cc)
			return string;
		
		string_cat_char(string, cc);
	}
	
	free_string(string);
	
	return NULL;
}


void gzwrite_string(gzFile file, struct string_t *string)
{
	char zero = 0;
	
	if(!string)
	{
		gzwrite(file, &zero, 1);
		return;
	}

	char *cc = string->text;
	
	while(*cc)
		gzwrite(file, cc++, 1);

	gzwrite(file, cc, 1);
}


struct string_t *gzread_string(gzFile file)
{
	struct string_t *string = new_string();
	
	char cc;
	
	while(gzread(file, &cc, 1) == 1)
	{
		if(!cc)
			return string;
		
		string_cat_char(string, cc);
	}
	
	free_string(string);
	
	return NULL;
}


void string_clear(struct string_t *string)
{
	if(!string)
		return;

	free(string->text);
	string->text = malloc(1);
	string->text[0] = '\0';
}



int string_isempty(struct string_t *string)
{
	if(!string)
		return 1;

	if(string->text[0] == '\0')
		return 1;

	return 0;
}
