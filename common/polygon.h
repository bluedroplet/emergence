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



struct polygon_t
{
	int numverts;
	struct vertex_t vertex[8];
};

void poly_line_clip(struct polygon_t *pin, struct vertex_t *v1, struct vertex_t *v2); // must be in c/w order
void poly_clip(struct polygon_t *pin, const struct polygon_t *pclip); // must be in c/w order
double poly_area(const struct polygon_t *poly); // must be in c/w order
double poly_clip_area(struct polygon_t *pin, const struct polygon_t *pclip); // must be in c/w order

void poly_arb_line_clip(struct vertex_ll_t **pin, struct vertex_t *v1, struct vertex_t *v2); // must be in c/w order
void poly_arb_clip(struct vertex_ll_t *pin, const struct vertex_ll_t *pclip); // must be in c/w order
double poly_arb_area(const struct vertex_ll_t *poly); // must be in c/w order
double poly_arb_clip_area(struct vertex_ll_t **pin, struct vertex_ll_t *pclip); // must be in c/w order
