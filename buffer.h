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


#ifndef _INC_NFBUFFER
#define _INC_NFBUFFER

#define BUF_EOB -1


struct buffer_t
{
	uint8_t *buf;
	int size;
	int writepos;
	int readpos;
};



struct buffer_t *new_buffer();
struct buffer_t *new_buffer_buffer(struct buffer_t *cat_buffer);
struct buffer_t *new_buffer_int(int val);

void free_buffer(struct buffer_t *buffer);

void buffer_cat_buffer(struct buffer_t *buffer, struct buffer_t *cat_buffer);
void buffer_cat_string(struct buffer_t *buffer, struct string_t *cat_string);
void buffer_cat_text(struct buffer_t *buffer, char *text);
void buffer_cat_buf(struct buffer_t *buffer, char *buf, int size);
void buffer_cat_int(struct buffer_t *buffer, int val);
void buffer_cat_uint64(struct buffer_t *buffer, uint64_t val);
void buffer_cat_double(struct buffer_t *buffer, double *val);
void buffer_cat_uint32(struct buffer_t *buffer, uint32_t val);
void buffer_cat_uint16(struct buffer_t *buffer, uint16_t val);
void buffer_cat_uint8(struct buffer_t *buffer, uint8_t val);
void buffer_cat_char(struct buffer_t *buffer, char val);

void buffer_setpos(struct buffer_t *buffer, int pos);
int buffer_more(struct buffer_t *buffer);

double buffer_read_double(struct buffer_t *buffer);
uint64_t buffer_read_uint64(struct buffer_t *buffer);
uint32_t buffer_read_uint32(struct buffer_t *buffer);
uint16_t buffer_read_uint16(struct buffer_t *buffer);
uint8_t buffer_read_uint8(struct buffer_t *buffer);
int buffer_read_int(struct buffer_t *buffer);
float buffer_read_float(struct buffer_t *buffer);
char buffer_read_char(struct buffer_t *buffer);
struct string_t *buffer_read_string(struct buffer_t *buffer);

void buffer_clear(struct buffer_t *buffer);


#endif // _INC_NFBUFFER
