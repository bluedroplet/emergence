/*
	Copyright (C) 1998-2002 Jonathan Brown
	
    This file is part of the gsub graphics library.
	
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


#ifndef _INC_GSUB
#define _INC_GSUB


#define SURFACE_1BIT				0
#define SURFACE_ALPHA8BIT			1
#define SURFACE_16BIT				2
#define SURFACE_16BITALPHA8BIT		3
#define SURFACE_24BIT				4
#define SURFACE_24BITALPHA8BIT		5
#define SURFACE_FLOATS				6
#define SURFACE_ALPHAFLOATS 		7
#define SURFACE_FLOATSALPHAFLOATS	8


struct surface_t
{
	int flags, width, height;
	void *buf;
	void *alpha_buf;
};

struct surface_t *new_surface(int flags, int width, int height);
void clear_surface(struct surface_t *surface);
void convert_surface_to_alpha8bit(struct surface_t *surface);
void convert_surface_to_16bit(struct surface_t *in);
void convert_surface_to_16bitalpha8bit(struct surface_t *surface);
void convert_surface_to_24bit(struct surface_t *surface);
void convert_surface_to_24bitalpha8bit(struct surface_t *surface);
void convert_surface_to_floats(struct surface_t *in);
struct surface_t *duplicate_surface(struct surface_t *in);
struct surface_t *duplicate_surface_to_24bit(struct surface_t *in);
void surface_flip_horiz(struct surface_t *surface);
void surface_flip_vert(struct surface_t *surface);
void surface_rotate_left(struct surface_t *surface);
void surface_rotate_right(struct surface_t *surface);
void surface_slide_horiz(struct surface_t *surface, int pixels);
void surface_slide_vert(struct surface_t *surface, int pixels);
void free_surface(struct surface_t *surface);

struct surface_t *read_png_surface(char *filename);
struct surface_t *read_png_surface_as_16bit(char *filename);
struct surface_t *read_png_surface_as_16bitalpha8bit(char *filename);
struct surface_t *read_png_surface_as_24bitalpha8bit(char *filename);
struct surface_t *read_png_surface_as_floats(char *filename);
struct surface_t *read_png_surface_as_floatsalphafloats(char *filename);
struct surface_t *read_raw_surface(char *filename);
int write_png_surface(struct surface_t *surface, char *filename);
int write_raw_surface(struct surface_t *surface, char *filename);


#ifdef _ZLIB_H
struct surface_t *gzread_raw_surface(gzFile file);
int gzwrite_raw_surface(gzFile file, struct surface_t *surface);
#endif


extern int (*gsub_callback)();



extern uint16_t *vid_backbuffer;
extern int vid_pitch;
extern int vid_width;
extern int vid_height;

void fb_update_mmx(void*, void*, int, int, int) __attribute__ ((cdecl));
void bb_clear_mmx(void*, int, int) __attribute__ ((cdecl));
void convert_16bit_to_32bit_mmx(void*, void*, int) __attribute__ ((cdecl));

void init_gsub();
void kill_gsub();

void clear_backbuffer();



extern uint8_t *vid_alphalookup;
extern uint16_t *vid_graylookup;
extern uint16_t *vid_redalphalookup;
extern uint16_t *vid_greenalphalookup;
extern uint16_t *vid_bluealphalookup;

extern int vid_bbsize, vid_width, vid_height, vid_heightm1,
	vid_byteswidth, vid_halfwidth, vid_halfheight, vid_halfheightm1;

extern uint16_t blit_colour;
extern uint8_t blit_alpha;


// blit_ops.cpp

extern struct surface_t *blit_source;

extern int blit_sourcex, blit_sourcey;
extern int blit_destx, blit_desty;
extern int blit_width, blit_height;


void plot_pixel();
void draw_rect();
void plot_alpha_pixel();
void draw_alpha_rect();
void blit_surface();
void blit_surface_rect();
void blit_alpha_surface();
void blit_alpha_surface_rect();
void alpha_blit_alpha_surface();
void alpha_blit_alpha_surface_rect();
void alpha_blit_surface();
void alpha_blit_surface_rect();
void alpha_surface_blit_surface();
void alpha_surface_blit_surface_rect();



// text.cpp

void init_text();


int blit_text(int x, int y, uint16_t colour, char *text);



// line.cpp

void draw_line(int x1, int y1, int x2, int y2);

struct surface_t *resize(struct surface_t *src_texture, int dst_width, int dst_height, int (*callback)());
struct surface_t *resizea(struct surface_t *src_texture, int dst_width, int dst_height, int (*callback)());
struct surface_t *crap_resize(struct surface_t *src_texture, int dst_width, int dst_height, int (*callback)());
struct surface_t *resize_16bitalpha8bit(struct surface_t *src_texture, int dst_width, int dst_height);
struct surface_t *rotate_surface(struct surface_t *in_surface, int scale_width, int scale_height, double theta);


#ifdef _INC_VERTEX

struct texture_verts_t
{
	struct surface_t *surface;
	struct vertex_t *verts;
	struct texture_verts_t *next;
};


struct texture_polys_t
{
	struct surface_t *surface;
	struct vertex_ll_t **polys;
	struct texture_polys_t *next;
};


struct surface_t *multiple_resample(struct texture_verts_t *texture_verts0, struct texture_polys_t *texture_polys0, 
	int dst_width, int dst_height, int dst_posx, int dst_posy, int (*callback)());

struct surface_t *resample(float *src_pixels, struct vertex_t *src_verts, int src_width, int src_height, 
			  int dst_width, int dst_height, int (*callback)());


#endif	// _INC_VERTEX

uint16_t convert_24bit_to_16bit(uint8_t red, uint8_t green, uint8_t blue);
//word convert_doubles_to_16bit(double red, double green, double blue);



#define convert_doubles_to_16bit(red, green, blue) \
((((uint16_t)(lround((red) * 31.0))) << 11) | \
((((uint16_t)(lround((green) * 63.0))) & 0x3f) << 5) | \
(((uint16_t)(lround((blue) * 31.0))) & 0x1f))

#define convert_double_to_8bit(alpha) ((uint8_t)lround((alpha) * 255.0))

#define get_double_red(in) ((double)((in) >> 11) / 31.0)
#define get_double_green(in) ((double)(((in) >> 5) & 0x3f) / 63.0)
#define get_double_blue(in) ((double)((in) & 0x1f) / 31.0)
#define get_double_from_8(in) ((double)in / 255.0)

#define get_float_red(in) ((float)((in) >> 11) / 31.0f)
#define get_float_green(in) ((float)(((in) >> 5) & 0x3f) / 63.0f)
#define get_float_blue(in) ((float)((in) & 0x1f) / 31.0f)
#define get_float_alpha(in) ((float)(in) / 255.0f)

#endif	// _INC_GSUB
