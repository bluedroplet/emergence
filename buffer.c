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

#ifdef WIN32
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "stringbuf.h"
#include "buffer.h"
#include "minmax.h"

#define BUFFER_GRANULARITY 4096


void buffer_ensure_space(struct buffer_t *buffer, int space)
{
	if(buffer->writepos + space > buffer->size)
	{
		buffer->size = ((buffer->writepos + space - 1) / BUFFER_GRANULARITY + 1) * BUFFER_GRANULARITY;
		buffer->buf = realloc(buffer->buf, buffer->size);
	}
}


struct buffer_t *new_buffer()
{
	struct buffer_t *buffer = malloc(sizeof(struct buffer_t));

	buffer->buf = NULL;
	buffer->size = 0;
	buffer->writepos = 0;
	buffer->readpos = 0;

	return buffer;
}


struct buffer_t *new_buffer_buffer(struct buffer_t *cat_buffer)
{
	struct buffer_t *buffer;
	
	if(!cat_buffer)
		return (void*)NULL;

	buffer = malloc(sizeof(struct buffer_t));

	buffer->size = cat_buffer->size;
	buffer->buf = malloc(buffer->size);
	buffer->writepos = cat_buffer->writepos;
	buffer->readpos = 0;

	memcpy(buffer->buf, cat_buffer->buf, buffer->writepos);

	return buffer;
}


struct buffer_t *new_buffer_int(int val)
{
	struct buffer_t *buffer = malloc(sizeof(struct buffer_t));

	buffer->buf = NULL;
	buffer->size = 0;
	buffer->writepos = 0;
	buffer->readpos = 0;

	buffer_cat_int(buffer, val);

	return buffer;
}


void free_buffer(struct buffer_t *buffer)
{
	if(!buffer)
		return;

	free(buffer->buf);
	free(buffer);
}


void buffer_cat_buffer(struct buffer_t *buffer, struct buffer_t *cat_buffer)
{
	if(!buffer)
		return;

	if(!cat_buffer)
		return;

	buffer_ensure_space(buffer, cat_buffer->writepos);
	
	memcpy(&buffer->buf[buffer->writepos], cat_buffer->buf, cat_buffer->writepos);
	buffer->writepos += cat_buffer->writepos;
}


void buffer_cat_string(struct buffer_t *buffer, struct string_t *cat_string)
{
	if(!buffer)
		return;

	if(!cat_string);
		return;

	int len = strlen(cat_string->text) + 1;
	
	buffer_ensure_space(buffer, len);
	
	memcpy(&buffer->buf[buffer->writepos], cat_string->text, len);
	buffer->writepos += len;
}


void buffer_cat_text(struct buffer_t *buffer, char *text)
{
	if(!buffer)
		return;

	if(!text)
		return;

	int len = strlen(text) + 1;
	
	buffer_ensure_space(buffer, len);
	
	memcpy(&buffer->buf[buffer->writepos], text, len);
	buffer->writepos += len;
}


void buffer_cat_buf(struct buffer_t *buffer, char *buf, int size)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, size);
	
	memcpy(&buffer->buf[buffer->writepos], buf, size);
	buffer->writepos += size;
}


void buffer_cat_int(struct buffer_t *buffer, int val)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, 4);
	
	*(int*)&(buffer->buf[buffer->writepos]) = val;
	buffer->writepos += 4;
}


void buffer_cat_uint64(struct buffer_t *buffer, uint64_t val)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, 8);
	
	*(uint64_t*)&(buffer->buf[buffer->writepos]) = val;
	buffer->writepos += 8;
}


void buffer_cat_double(struct buffer_t *buffer, double *val)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, 8);
	
	*(double*)&(buffer->buf[buffer->writepos]) = *val;
	buffer->writepos += 8;
}


void buffer_cat_uint32(struct buffer_t *buffer, uint32_t val)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, 4);
	
	*(uint32_t*)&(buffer->buf[buffer->writepos]) = val;
	buffer->writepos += 4;
}


void buffer_cat_uint16(struct buffer_t *buffer, uint16_t val)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, 2);
	
	*(uint16_t*)&(buffer->buf[buffer->writepos]) = val;
	buffer->writepos += 2;
}


void buffer_cat_uint8(struct buffer_t *buffer, uint8_t val)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, 1);
	
	*(uint8_t*)&(buffer->buf[buffer->writepos++]) = val;
}


void buffer_cat_char(struct buffer_t *buffer, char val)
{
	if(!buffer)
		return;

	buffer_ensure_space(buffer, 1);
	
	*(char*)&(buffer->buf[buffer->writepos++]) = val;
}


/*
void buffer_setpos(struct buffer_t *buffer, int pos)
{
	if(!buffer)
		return;

	buffer->pos = pos;
}
*/

int buffer_more(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;
	
	return(max(0, buffer->writepos - buffer->readpos));
}


double buffer_read_double(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;

	double val;

	if(buffer_more(buffer) >= 8)
	{
		val = *(double*)&buffer->buf[buffer->readpos];
		buffer->readpos += 8;
	}
	else
		val = (double)BUF_EOB;
	
	return val;
}


uint64_t buffer_read_uint64(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;

	uint64_t val;

	if(buffer_more(buffer) >= 8)
	{
		val = *(uint64_t*)&buffer->buf[buffer->readpos];
		buffer->readpos += 8;
	}
	else
		val = (uint64_t)BUF_EOB;
	
	return val;
}


uint32_t buffer_read_uint32(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;

	uint32_t val;

	if(buffer_more(buffer) >= 4)
	{
		val = *(uint32_t*)&buffer->buf[buffer->readpos];
		buffer->readpos += 4;
	}
	else
		val = (uint32_t)BUF_EOB;
	
	return val;
}


uint16_t buffer_read_uint16(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;

	uint16_t val;

	if(buffer_more(buffer) >= 2)
	{
		val = *(uint16_t*)&buffer->buf[buffer->readpos];
		buffer->readpos += 2;
	}
	else
		val = (uint16_t)BUF_EOB;
	
	return val;
}




uint8_t buffer_read_uint8(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;

	uint8_t val;

	if(buffer_more(buffer) >= 1)
		val = *(uint8_t*)&buffer->buf[buffer->readpos++];
	else
		val = (uint8_t)BUF_EOB;
	
	return val;
}


int buffer_read_int(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;

	int val;

	if(buffer_more(buffer) >= 4)
	{
		val = *(int*)&buffer->buf[buffer->readpos];
		buffer->readpos += 4;
	}
	else
		val = (int)BUF_EOB;

	return val;
}




float buffer_read_float(struct buffer_t *buffer)
{
	if(!buffer)
		return 0.0F;

	float val;

	if(buffer_more(buffer) >= 4)
	{
		val = *(float*)&buffer->buf[buffer->readpos];
		buffer->readpos += 4;
	}
	else
		val = 0.0f;
	
	return val;
}


char buffer_read_char(struct buffer_t *buffer)
{
	if(!buffer)
		return 0;

	if(buffer_more(buffer) >= 1)
		return buffer->buf[buffer->readpos++];

	return (char)BUF_EOB;
}



struct string_t *buffer_read_string(struct buffer_t *buffer)
{
	if(!buffer)
		return NULL;

	struct string_t *s = new_string();

	while(buffer_more(buffer))
	{
		char c = buffer->buf[buffer->readpos++];
		
		if(c == 0)
			break;

		string_cat_char(s, c);
	}

	return s;
}



void buffer_clear(struct buffer_t *buffer)
{
	if(!buffer)
		return;

	buffer->readpos = 0;
	buffer->writepos = 0;
}
