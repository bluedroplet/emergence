/*
	Copyright (C) 1998-2002 Jonathan Brown
	
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




#ifndef _INC_NFSTRING
#define _INC_NFSTRING


struct string_t
{
	char *text;
};

struct string_t *new_string();
struct string_t *new_string_string(struct string_t *cat_string);
struct string_t *new_string_text(const char *fmt, ...);
struct string_t *new_string_char(char c);
struct string_t *new_string_int(int val);
struct string_t *new_string_uint32(uint32_t val);
struct string_t *new_string_double(double val, int digits);
struct string_t *new_string_uint64(uint64_t val);

void free_string(struct string_t *string);

void string_cat_string(struct string_t *string, struct string_t *cat_string);
void string_cat_text(struct string_t *string, const char *fmt, ...);
void string_cat_char(struct string_t *string, char c);
void string_cat_int(struct string_t *string, int val);
void string_cat_uint32(struct string_t *string, uint32_t val);
void string_cat_double(struct string_t *string, double val, int digits);
void string_cat_uint64(struct string_t *string, uint64_t val);

#if defined _INC_STDIO || defined _STDIO_H
void fwrite_string(struct string_t *string, FILE *file);
struct string_t *fread_string(FILE *file);
#endif

#ifdef _ZLIB_H
void gzwrite_string(gzFile file, struct string_t *string);
struct string_t *gzread_string(gzFile file);
#endif

int string_isempty(struct string_t *string);
void string_clear(struct string_t *string);


#endif
